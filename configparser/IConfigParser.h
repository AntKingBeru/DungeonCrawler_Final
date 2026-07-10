#ifndef ICONFIG_PARSER_H
#define ICONFIG_PARSER_H

#include <string>
#include "ConfigData.h"

class IConfigParser
{
public:
	virtual ~IConfigParser() = default;
	virtual ConfigData parse(const std::string& path) = 0;
	virtual void serialize(const ConfigData& data, const std::string& path) = 0;
};

#endif