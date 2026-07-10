#include "IniParser.h"
#include "StringUtils.h"
#include <fstream>
#include <stdexcept>
#include <cctype>

static std::string stripInlineComment(const std::string& value)
{
    for (size_t i = 0; i < value.size(); ++i)
        if ((value[i] == ';' || value[i] == '#') &&
            (i == 0 || std::isspace(static_cast<unsigned char>(value[i - 1]))))
            return value.substr(0, i);
    return value;
}

ConfigData IniParser::parse(const std::string& path)
{
    std::ifstream in(path);
    if (!in)
        throw std::runtime_error("Cannot open file: " + path);

    ConfigData data;
    std::string currentSection;
    std::string line;

    while (std::getline(in, line))
    {
        std::string t = strutil::trim(line);
        if (t.empty())
            continue;
        if (t[0] == '#' || t[0] == ';')
            continue;

        if (t.front() == '[' && t.back() == ']')
        {
            currentSection = strutil::trim(t.substr(1, t.size() - 2));
            data[currentSection];
            continue;
        }

        size_t eq = t.find('=');
        if (eq == std::string::npos)
            throw std::runtime_error("Malformed INI line: " + line);

        std::string key = strutil::trim(t.substr(0, eq));
        std::string val = strutil::trim(stripInlineComment(t.substr(eq + 1)));
        data[currentSection][key] = val;
    }
    return data;
}

void IniParser::serialize(const ConfigData& data, const std::string& path)
{
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Cannot write file: " + path);

    for (const auto& [section, kvs] : data) {
        out << '[' << section << "]\n";
        for (const auto& [key, val] : kvs)
            out << key << " = " << val << '\n';
        out << '\n';
    }
}