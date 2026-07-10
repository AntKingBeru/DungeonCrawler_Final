#include "JsonParser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <cstdlib>

namespace
{
    std::string readAll(const std::string& path)
    {
        std::ifstream in(path);
        if (!in)
            throw std::runtime_error("Cannot open file: " + path);
        std::stringstream ss; ss << in.rdbuf(); return ss.str();
    }

    void skipWs(const std::string& s, size_t& i)
    {
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
            i++;
    }

    std::string parseString(const std::string& s, size_t& i)
    {
        if (i >= s.size() || s[i] != '"')
            throw std::runtime_error("JSON: expected string");
        i++;
        std::string out;
        while (i < s.size() && s[i] != '"')
        {
            char c = s[i++];
            if (c == '\\' && i < s.size())
            {
                char e = s[i++];
                switch (e)
                {
                    case 'n':
                        out += '\n';
                        break;
                    case 't':
                        out += '\t';
                        break;
                    case 'r':
                        out += '\r';
                        break;
                    case 'b':
                        out += '\b';
                        break;
                    case 'f':
                        out += '\f';
                        break;
                    case '/':
                        out += '/';
                        break;
                    case '"':
                        out += '"';
                        break;
                    case '\\':
                        out += '\\';
                        break;
                    default:
                        out += e;
                        break;
                }
            }
            else
                out += c;
        }
        if (i >= s.size())
            throw std::runtime_error("JSON: unterminated string");
        i++;
        return out;
    }

    std::string parseScalar(const std::string& s, size_t& i)
    {
        skipWs(s, i);
        if (i >= s.size())
            throw std::runtime_error("JSON: unexpected end of input");
        if (s[i] == '"')
            return parseString(s, i);
        size_t start = i;
        while (i < s.size() && s[i] != ',' && s[i] != '}' && s[i] != ']' &&
            !std::isspace(static_cast<unsigned char>(s[i])))
            i++;
        if (i == start)
            throw std::runtime_error("JSON: empty value");
        return s.substr(start, i - start);
    }

    bool looksLikeNumber(const std::string& v)
    {
        if (v.empty())
            return false;
        char* end = nullptr;
        std::strtod(v.c_str(), &end);
        return end == v.c_str() + v.size();
    }

    std::string jsonEscape(const std::string& v)
    {
        std::string out;
        for (char c : v) switch (c)
        {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\t':
                out += "\\t";
                break;
            case '\r':
                out += "\\r";
                break;
            default:
                out += c;
                break;
        }
        return out;
    }

    std::string formatValue(const std::string& v)
    {
        if (v == "true" || v == "false" || v == "null")
            return v;
        if (looksLikeNumber(v))
            return v;
        return "\"" + jsonEscape(v) + "\"";
    }
}

ConfigData JsonParser::parse(const std::string& path)
{
    std::string s = readAll(path);
    size_t i = 0;
    ConfigData data;

    skipWs(s, i);
    if (i >= s.size() || s[i] != '{')
        throw std::runtime_error("JSON: root must be an object");
    i++;
    skipWs(s, i);
    if (i < s.size() && s[i] == '}')
        return data;

    while (i < s.size())
    {
        skipWs(s, i);
        std::string section = parseString(s, i);
        skipWs(s, i);
        if (i >= s.size() || s[i] != ':')
            throw std::runtime_error("JSON: expected ':'");
        i++;
        skipWs(s, i);
        if (i >= s.size() || s[i] != '{')
            throw std::runtime_error("JSON: section '" + section + "' must be an object");
        i++;

        skipWs(s, i);
        if (i < s.size() && s[i] == '}')
        {
            i++;
            data[section];
        }
        else
        {
            while (i < s.size())
            {
                skipWs(s, i);
                std::string key = parseString(s, i);
                skipWs(s, i);
                if (i >= s.size() || s[i] != ':')
                    throw std::runtime_error("JSON: expected ':'");
                i++;
                data[section][key] = parseScalar(s, i);
                skipWs(s, i);
                if (i < s.size() && s[i] == ',')
                {
                    i++;
                    continue;
                }
                if (i < s.size() && s[i] == '}')
                {
                    i++;
                    break;
                }
                throw std::runtime_error("JSON: expected ',' or '}' in section");
            }
        }

        skipWs(s, i);
        if (i < s.size() && s[i] == ',')
        {
            i++;
            continue;
        }
        if (i < s.size() && s[i] == '}')
        {
            i++;
            break;
        }
        throw std::runtime_error("JSON: expected ',' or '}' at top level");
    }
    return data;
}

void JsonParser::serialize(const ConfigData& data, const std::string& path)
{
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Cannot write file: " + path);

    out << "{\n";
    size_t si = 0;
    for (const auto& [section, kvs] : data)
    {
        out << "  \"" << jsonEscape(section) << "\": {\n";
        size_t ki = 0;
        for (const auto& [key, val] : kvs)
        {
            out << "    \"" << jsonEscape(key) << "\": " << formatValue(val)
                << (++ki < kvs.size() ? ",\n" : "\n");
        }
        out << "  }" << (++si < data.size() ? ",\n" : "\n");
    }
    out << "}\n";
}