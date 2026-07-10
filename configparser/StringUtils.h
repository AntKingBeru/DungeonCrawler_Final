#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <utility>

namespace strutil
{
	std::string trim(const std::string& s);
	std::string toLower(const std::string& s);
	std::pair<std::string, std::string> splitFirst(const std::string& s, char delim);
}

#endif