#pragma once
#include <vector>
#include <string>

namespace string_utility
{
	std::vector<std::string> string_split(const std::string& s, std::string&& c);
	bool startsWith(const char* src, const char* des);
	bool endsWith(const char* src, const char* des);
}
