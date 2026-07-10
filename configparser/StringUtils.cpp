#include "StringUtils.h"
#include <algorithm>
#include <cctype>

namespace strutil
{

    std::string trim(const std::string& s)
    {
        size_t start = 0, end = s.size();
        while (start < end && std::isspace(static_cast<unsigned char>(s[start])))
            start++;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            end--;
        return s.substr(start, end - start);
    }

    std::string toLower(const std::string& s)
    {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
            [](unsigned char c)
            { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    std::pair<std::string, std::string> splitFirst(const std::string& s, char delim)
    {
        size_t pos = s.find(delim);
        if (pos == std::string::npos)
            return { s, "" };
        return { s.substr(0, pos), s.substr(pos + 1) };
    }
}