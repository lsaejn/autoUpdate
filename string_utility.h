#pragma once
#include <vector>
#include <string>

namespace string_utility
{
	std::vector<std::string> string_split(const std::string& s, std::string&& c)
	{
		std::vector<std::string> result;
		size_t len = s.length();
		std::string::size_type pos1 = 0;
		std::string::size_type pos2 = s.find(c);
		while (std::string::npos != pos2)
		{
			result.push_back(s.substr(pos1, pos2 - pos1));
			pos1 = pos2 + c.size();
			pos2 = s.find(c, pos1);
		}
		if (pos1 != len)
			result.push_back(s.substr(pos1));
		return result;
	}


	bool startsWith(const char* src, const char* des)
	{
		return (strlen(des) <= strlen(src)) &&
			(!memcmp(src, des, strlen(des)));
	}

	bool endsWith(const char* src, const char* des)
	{
		int offset = strlen(src) - strlen(des);
		return  offset >= 0
			&& startsWith(src + offset, des);
	}
}
