#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "IConfigParser.h"

class JsonParser : public IConfigParser
{
public:
    ConfigData parse(const std::string& path)
        override;
    void serialize(const ConfigData& data, const std::string& path)
        override;
};

#endif