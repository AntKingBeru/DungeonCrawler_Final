#ifndef PARSER_FACTORY_H
#define PARSER_FACTORY_H

#include <memory>
#include <string>
#include "IConfigParser.h"

std::string extensionOf(const std::string& path);

std::unique_ptr<IConfigParser> makeParser(const std::string& path);

#endif