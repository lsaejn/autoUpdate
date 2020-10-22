#pragma once

#include <string>
#include <algorithm>
#include <regex>

#include "Alime/processIterator.h"
#include "AppUtility.h"

/*
fix me, 大量代码重复，没有时间重写
*/
class ProcessManager
{
public:
	ProcessManager() = default;

	void SetMatchReg(const std::wstring& name)
	{
		matchName_ = name;
	}

	bool AssurePkpmmainClosed()
	{
		using Alime::ProcessIterator;

		bool stop = false;
		//fix me, function GetFileApart()
		while (!stop)
		{
			ProcessIterator iter;
			while (iter.hasNext())
			{
				auto fullPath = iter.getPath();
				++iter;
				int index = -1;
				for (size_t i = 0; i != fullPath.size(); ++i)
				{
					if (fullPath[i] == L'\\' || fullPath[i] == L'/')
					{
						index = i;
					}
				}
				std::wstring fileName = fullPath.substr(index + 1);
				std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](wchar_t ch) {
					if (ch >= L'a' && ch <= L'z')
						ch += L'A' - L'a';
					return ch;
					});

				if (fileName == matchName_)
				{
					auto ret = ShowQuestionBox(u8"检测到PKPM相关程序正在工作",
						u8"更新操作需要关闭所有的PKPM相关程序，请关闭程序后点击继续，请选择操作",
						u8"继续",
						u8"取消");
					if (ret)
						break;
					else
						return false;
				}
			}
			return true;
		}
		return false;
	}

	bool ShutDownExistingApp()
	{
		using Alime::ProcessIterator;

		bool customNotified = false;
		bool stop = false;
		ProcessIterator iter;
		while (iter.hasNext() && !stop)
		{
			auto fullPath=iter.getPath();
			int index = -1;
			for (size_t i = 0; i != fullPath.size(); ++i)
			{
				if (fullPath[i] == L'\\' || fullPath[i] == L'/')
				{
					index = i;
				}
			}
			std::wstring fileName= fullPath.substr(index+1);
			std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](wchar_t ch) {
				if (ch >= L'a' && ch <= L'z')
					ch += L'A' - L'a';
				return ch;
				});
			
			if (fileName == matchName_)
			{
				if (!customNotified)
				{
					auto ret = ShowQuestionBox(u8"检测到PKPM相关程序正在工作",
						u8"更新操作需要关闭所有的PKPM相关程序，请关闭程序后点击继续，请选择操作",
						u8"继续",
						u8"取消");
					if (ret)
					{
						iter.KillProcess(iter->th32ProcessID);
						qDebug() << "user should close app";
					}
					else
					{
						stop = true;
						break;
						qDebug() << "user stopped setup";
					}
					customNotified = true;
				}
				iter.KillProcess(iter->th32ProcessID);
			}
			++iter;
		}
		return !stop;
	}


	//返回操作是否被用户中断
	//fix me， 重复的代码
	bool ShutDownFuzzyMatchApp()
	{
		using Alime::ProcessIterator;

		bool customNotified = false;
		bool stop = false;
		ProcessIterator iter;
		while (iter.hasNext() && !stop)
		{
			auto fullPath = iter.getPath();
			int index = -1;
			for (size_t i = 0; i != fullPath.size(); ++i)
			{
				if (fullPath[i] == L'\\' || fullPath[i] == L'/')
				{
					index = i;
				}
			}
			std::wstring fileName = fullPath.substr(index + 1);
			std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](wchar_t ch) {
				if (ch >= L'a' && ch <= L'z')
					ch += L'A' - L'a';
				return ch;
				});

			//
			std::wregex regx(matchName_);
			bool matched=std::regex_search(fileName, regx);
			if (matched)
			{
				if (!customNotified)
				{
					auto ret = ShowQuestionBox(u8"检测到PKPM启动程序正在工作",
						u8"更新操作需要关闭所有的PKPM相关程序，点击\"继续\"将强制关闭程序，请选择操作",
						u8"继续",
						u8"取消");
					if (ret)
					{
						iter.KillProcess(iter->th32ProcessID);
						qDebug() << "user close app";
					}
					else
					{
						stop = true;
						qDebug() << "user stopped setup";
					}
					customNotified = true;
				}
			}
			++iter;
		}
		return !stop;
	}

private:
	std::wstring matchName_;
};
