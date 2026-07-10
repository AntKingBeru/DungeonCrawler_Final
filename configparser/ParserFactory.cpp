#include "ParserFactory.h"
#include "StringUtils.h"
#include "IniParser.h"
#include "JsonParser.h"
#include "XmlParser.h"
#include <stdexcept>

std::string extensionOf(const std::string& path)
{
    size_t dot = path.find_last_of('.');
    size_t slash = path.find_last_of("/\\");
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return "";
    return strutil::toLower(path.substr(dot + 1));
}

std::unique_ptr<IConfigParser> makeParser(const std::string& path)
{
    std::string ext = extensionOf(path);
    if (ext == "ini")
        return std::make_unique<IniParser>();
    if (ext == "json")
        return std::make_unique<JsonParser>();
    if (ext == "xml")
        return std::make_unique<XmlParser>();
    throw std::runtime_error("Unsupported format: " + (ext.empty() ? path : ext));
}