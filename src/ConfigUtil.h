#ifndef CONFIG_UTIL_H
#define CONFIG_UTIL_H

#include <string>
#include <stdexcept>
#include "ConfigData.h"

namespace cfg
{
    inline std::string require(const ConfigData& data, const std::string& section, const std::string& key)
    {
        auto s = data.find(section);
        if (s == data.end())
            throw std::runtime_error("Config missing section [" + section + "]");
        auto k = s->second.find(key);
        if (k == s->second.end())
            throw std::runtime_error("Config missing key '" + key + "' in [" + section + "]");
        return k->second;
    }

    inline int requireInt(const ConfigData& data, const std::string& section, const std::string& key)
    {
        return std::stoi(require(data, section, key));
    }

    inline int intOr(const ConfigData& data, const std::string& section, const std::string& key, int fallback)
    {
        auto s = data.find(section);
        if (s == data.end())
            return fallback;
        auto k = s->second.find(key);
        if (k == s->second.end())
            return fallback;
        try
        {
            return std::stoi(k->second);
        }
        catch (...)
        {
            return fallback;
        }
    }
}

#endif