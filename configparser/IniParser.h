#ifndef INI_PARSER_H
#define INI_PARSER_H

#include "IConfigParser.h"

class IniParser : public IConfigParser
{
public:
	ConfigData parse(const std::string& path)
		override;
	void serialize(const ConfigData& data, const std::string& path)
		override;
};

#endif