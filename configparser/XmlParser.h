#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "IConfigParser.h"

class XmlParser : public IConfigParser
{
public:
    ConfigData parse(const std::string& path)
        override;
    void serialize(const ConfigData& data, const std::string& path)
        override;
};

#endif