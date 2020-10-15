#pragma once
#include <vector>
#include <string>

namespace string_utility
{
	std::vector<std::string> string_split(const std::string& s, const std::string& c);
	bool startsWith(const char* src, const char* des);
	bool startsWith(const char* src, const char* des, int len);
	bool endsWith(const char* src, const char* des);
}
