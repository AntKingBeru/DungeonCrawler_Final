#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <map>
#include <string>

using Section = std::map<std::string, std::string>;

using ConfigData = std::map<std::string, Section>;

#endif