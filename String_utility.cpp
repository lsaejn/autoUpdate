#include "String_utility.h"

namespace string_utility
{
	std::vector<std::string> string_split(const std::string& s1, const std::string& c)
	{
		std::string s;
		if (startsWith(s1.c_str(), "V") || startsWith(s1.c_str(), "v"))
			s = s1.substr(1);
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
		return (std::strlen(des) <= std::strlen(src)) &&
			(!std::memcmp(src, des, std::strlen(des)));
	}

	bool startsWith(const char* src, const char* des, int len)
	{
		size_t len1 = std::strlen(src);
		size_t len2 = std::strlen(des);
		if (len1 < len || len2 < len)
			return false;
		return !std::memcmp(src, des, len);
	}

	bool endsWith(const char* src, const char* des)
	{
		int offset = std::strlen(src) - std::strlen(des);
		return  offset >= 0
			&& startsWith(src + offset, des);
	}
}