#include "XmlParser.h"
#include "StringUtils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cctype>

namespace
{
    struct XmlNode
    {
        std::string name;
        std::string text;
        std::vector<XmlNode> children;
    };

    std::string readAll(const std::string& path)
    {
        std::ifstream in(path);
        if (!in)
            throw std::runtime_error("Cannot open file: " + path);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    std::string stripMeta(std::string s)
    {
        size_t p;
        while ((p = s.find("<?")) != std::string::npos)
        {
            size_t e = s.find("?>", p);
            if (e == std::string::npos)
                break;
            s.erase(p, e - p + 2);
        }
        while ((p = s.find("<!--")) != std::string::npos)
        {
            size_t e = s.find("-->", p);
            if (e == std::string::npos)
                break;
            s.erase(p, e - p + 3);
        }
        return s;
    }

    std::string xmlUnescape(const std::string& s)
    {
        std::string out;
        for (size_t i = 0; i < s.size();)
        {
            if (s[i] == '&') {
                if (s.compare(i, 5, "&amp;") == 0)
                {
                    out += '&';
                    i += 5;
                }
                else if (s.compare(i, 4, "&lt;") == 0)
                {
                    out += '<';
                    i += 4;
                }
                else if (s.compare(i, 4, "&gt;") == 0)
                {
                    out += '>';
                    i += 4;
                }
                else if (s.compare(i, 6, "&quot;") == 0)
                {
                    out += '"';
                    i += 6;
                }
                else if (s.compare(i, 6, "&apos;") == 0)
                {
                    out += '\'';
                    i += 6;
                }
                else
                {
                    out += s[i++];
                }
            }
            else
                out += s[i++];
        }
        return out;
    }

    std::string xmlEscape(const std::string& s)
    {
        std::string out;
        for (char c : s) switch (c)
        {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '"':
                out += "&quot;";
                break;
            case '\'':
                out += "&apos;";
                break;
            default:
                out += c;
                break;
        }
        return out;
    }

    void skipWs(const std::string& s, size_t& i)
    {
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
            i++;
    }

    XmlNode parseNode(const std::string& s, size_t& i)
    {
        skipWs(s, i);
        if (i >= s.size() || s[i] != '<')
            throw std::runtime_error("XML: expected '<'");
        i++;

        XmlNode node;
        while (i < s.size() && s[i] != '>' && s[i] != '/' &&
            !std::isspace(static_cast<unsigned char>(s[i]))) node.name += s[i++];
        if (node.name.empty())
            throw std::runtime_error("XML: empty tag name");

        bool selfClose = false;
        while (i < s.size() && s[i] != '>')
        {
            if (s[i] == '/')
                selfClose = true;
            i++;
        }
        if (i >= s.size())
            throw std::runtime_error("XML: unclosed tag <" + node.name + ">");
        i++;
        if (selfClose)
            return node;

        std::string textAccum;
        while (i < s.size())
        {
            if (s[i] == '<')
            {
                if (i + 1 < s.size() && s[i + 1] == '/')
                {
                    i += 2;
                    std::string closeName;
                    while (i < s.size() && s[i] != '>')
                        closeName += s[i++];
                    if (i < s.size())
                        i++;
                    if (strutil::trim(closeName) != node.name)
                        throw std::runtime_error("XML: mismatched </" + strutil::trim(closeName) +
                            "> for <" + node.name + ">");
                    node.text = strutil::trim(xmlUnescape(textAccum));
                    return node;
                }
                node.children.push_back(parseNode(s, i));
            }
            else
                textAccum += s[i++];
        }
        throw std::runtime_error("XML: unexpected end inside <" + node.name + ">");
    }
}

ConfigData XmlParser::parse(const std::string& path)
{
    std::string s = stripMeta(readAll(path));
    size_t i = 0;
    skipWs(s, i);
    if (i >= s.size())
        throw std::runtime_error("XML: empty document");

    XmlNode root = parseNode(s, i);

    ConfigData data;
    for (const auto& section : root.children)
    {
        data[section.name];
        for (const auto& kv : section.children)
            data[section.name][kv.name] = kv.text;
    }
    return data;
}

void XmlParser::serialize(const ConfigData& data, const std::string& path)
{
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Cannot write file: " + path);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<config>\n";
    for (const auto& [section, kvs] : data)
    {
        out << "  <" << section << ">\n";
        for (const auto& [key, val] : kvs)
            out << "    <" << key << ">" << xmlEscape(val) << "</" << key << ">\n";
        out << "  </" << section << ">\n";
    }
    out << "</config>\n";
}