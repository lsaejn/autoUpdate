#pragma once
#include <vector>
#include <string>
#include <io.h>
#include <QString>
#include "string_utility.h"


class FindSpecificFiles
{
public:
	static std::vector<std::string> FindVersionFiles(const char* path, const char* prefix, const char* suffix)
	{
		std::string toFind(path);
		toFind += "\\*.";
		toFind += suffix;
		std::vector<std::string> result;
		long handle;
		_finddata_t fileinfo;
		handle = _findfirst(toFind.c_str(), &fileinfo);
		if (handle == -1)
			return result;
		do
		{
			if(string_utility::startsWith(fileinfo.name, prefix)&& string_utility::endsWith(fileinfo.name, suffix))
			{
				std::string filename(fileinfo.name);
				filename = filename.substr(1, filename.size() - 5);
				if (IsValidVersionName(filename))
					result.push_back(filename);
			}
		} while (!_findnext(handle, &fileinfo));
		_findclose(handle);
		return result;
	}
private:
	static bool IsValidVersionName(const std::string& str)
	{
		if (!isdigit(str.front()) || str.find("..") != std::string::npos)
			return false;
		for (auto elem : str)
		{
			if (!isdigit(elem) && elem != '.')
				return false;
		}
		return true;
	}
};

class AscendingOrder
{
public:
	bool operator()(const std::string& lhs, const std::string& rhs)
	{
		auto copyLhs = lhs;
		auto copyRhs = rhs;
		CheckPreFix(copyLhs, copyRhs);
		int ret = Compare(copyLhs, copyRhs);
		if (ret == -1)
			return true;
		return false;
	}

	void CheckPreFix(std::string& lhs, std::string& rhs)
	{
		if (lhs.front() == 'V' && rhs.front() == 'V')
		{
			lhs = lhs.substr(1);
			rhs = rhs.substr(1);
		}
	}

	//@return -1 when lhs<rhs , 0 when lhs==rhs, 1 when lhs>rhs
	int Compare(const std::string& _lhs, const std::string& _rhs)
	{
		std::string lhs = _lhs;
		std::string rhs = _rhs;
		//fix me, CheckPreFix
		if (string_utility::startsWith(lhs.c_str(), "V") || string_utility::startsWith(lhs.c_str(), "v"))
			lhs = lhs.substr(1);
		if (string_utility::startsWith(rhs.c_str(), "V") || string_utility::startsWith(rhs.c_str(), "v"))
			rhs = rhs.substr(1);
		auto lhsElems = StringsToIntegers(string_utility::string_split(lhs, "."));
		auto rhsElems = StringsToIntegers(string_utility::string_split(rhs, "."));
		TrimTailZeros(lhsElems); TrimTailZeros(rhsElems);
		auto numToCompare = lhsElems.size() <= rhsElems.size() ? lhsElems.size() : rhsElems.size();
		for (size_t i = 0; i != numToCompare; ++i)
		{
			if (lhsElems[i] != rhsElems[i])
				return lhsElems[i] < rhsElems[i] ? -1 : 1;
		}
		if (lhsElems.size() == rhsElems.size())
			return 0;
		return lhsElems.size() < rhsElems.size() ? -1 : 1;
	}

	std::vector<int> StringsToIntegers(const std::vector<std::string>& strs)
	{
		std::vector<int> numCol;
		for (size_t i = 0; i != strs.size(); ++i)
			numCol.push_back(std::stoi(strs[i]));
		return numCol;
	}

	void TrimTailZeros(std::vector<int>& nums)
	{
		int i = nums.size() - 1;
		for (; i >= 0; --i)
			if (nums[i] != 0)
				break;
		nums.resize(i + 1);
	}
};

