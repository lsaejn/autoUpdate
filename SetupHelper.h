#pragma once

#include <string>
#include <algorithm>
#include <regex>
#include <map>
#include <set>

#include "Alime/processIterator.h"
#include "Alime/ScopeGuard.h"

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

		bool findPkpmApp = false;
		//fix me, function GetFileApart()
		while (1)
		{
			ProcessIterator iter;
			findPkpmApp = false;
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
					findPkpmApp = true;
					auto ret = ShowQuestionBox(u8"检测到PKPM相关程序正在工作",
						u8"更新操作需要关闭所有的PKPM相关程序，请关闭程序后点击继续，请选择操作",
						u8"继续",
						u8"取消");
					if (ret)
						continue;
					else
						return false;
				}
			}
			if (!findPkpmApp)
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
		while (!stop)
		{
			while (iter.hasNext())
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
		}
		return !stop;
	}
	//返回操作是否被用户中断, 
	//true 用户点击确定，即所有启动界面被关闭
	//false 用户取消
	//fix me， 重复的代码
	bool ShutDownFuzzyMatchApp()
	{
		//fix me,去掉
		if (g_hwnd && IsWindow(g_hwnd))
		{
			SendMessage(g_hwnd, WM_USER + 7, 0, 0);
		}

		TryToCloseAllFuzzyMatchApp();
		while(true)
		{
			Alime::ProcessIterator iter;
			auto set=FindHwndByMatchName();
			if (!set.empty())
			{
				auto ret = ShowQuestionBox(u8"检测到PKPM启动程序正在工作",
					u8"更新操作需要关闭所有的PKPM启动程序，请手动关闭后点击\"继续\"，请选择操作",
					u8"继续",
					u8"取消");
				if (ret)
				{
					//做善事。再帮用户关一次
					TryToCloseAllFuzzyMatchApp();
				}
				else
				{
					return false;
				}
			}
			else
			{
				break;
			}

		}
		return true;
	}

	std::set<HWND> FindHwndByMatchName()
	{
		std::set<HWND> ret;
		Alime::ProcessIterator iter;
		while (iter.hasNext())
		{
			auto fullPath = iter.getPath();
			std::wstring fileName = GetFileNameUpper(fullPath);
			std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](wchar_t ch) {
				if (ch >= L'a' && ch <= L'z')
					ch += L'A' - L'a';
				return ch;
				});

			std::wregex regx(matchName_);
			if (std::regex_search(fileName, regx))
			{
				auto path = Alime::ProcessIterator::GetAppFullPathByPid(iter->th32ProcessID);
				path = GetParentFolderW(path);
				if (GetExeFolderW() == path)
				{
					auto hwnd = Alime::ProcessIterator::FindMainHwndByPid(iter->th32ProcessID);
					ret.insert(hwnd);
				}
			}
			++iter;
		}
		return ret;
	}

	std::wstring GetFileNameUpper(const std::wstring& fullPath)
	{
		int index = -1;
		for (size_t i = 0; i != fullPath.size(); ++i)
		{
			if (fullPath[i] == L'\\' || fullPath[i] == L'/')
			{
				index = i;
			}
		}
		std::wstring fileName = fullPath.substr(index + 1);
		return fileName;
	}

	//关一遍能关的
	void TryToCloseAllFuzzyMatchApp()
	{
		using Alime::ProcessIterator;
		ProcessIterator iter;
		while (iter.hasNext())
		{
			auto fullPath = iter.getPath();
			std::wstring fileName = GetFileNameUpper(fullPath);
			std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](wchar_t ch) {
				if (ch >= L'a' && ch <= L'z')
					ch += L'A' - L'a';
				return ch;
				});

			std::wregex regx(matchName_);
			if (std::regex_search(fileName, regx))
			{
				auto path = Alime::ProcessIterator::GetAppFullPathByPid(iter->th32ProcessID);
				path = GetParentFolderW(path);
				auto another= GetParentFolderW(GetExeFolderW());
				if (another == path)
				{
					auto hwnd = Alime::ProcessIterator::FindMainHwndByPid(iter->th32ProcessID);
					SendMessage(hwnd, WM_USER + 7, 0, 0);
				}
			}
			++iter;
		}
	}

private:
	std::wstring matchName_;
};
