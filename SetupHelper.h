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
	//true 保证用户点击关闭相关程序
	//false 用户取消
	//fix me， 重复的代码
	bool ShutDownFuzzyMatchApp()
	{
		using Alime::ProcessIterator;

		bool customNotified = false;
		bool findApp = false;
		//
		
		while (1)//用户可能开完升级器之后把启动界面关闭
		{
			if (g_hwnd&& IsWindow(g_hwnd))
			{
				SendMessage(g_hwnd, WM_USER + 7, 0, 0);
			}
			ProcessIterator iter;
			findApp = false;
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

				std::wregex regx(matchName_);
				findApp = std::regex_search(fileName, regx);
				if (findApp)
				{
					auto ret = ShowQuestionBox(u8"检测到PKPM启动程序正在工作",
						u8"更新操作需要关闭所有的PKPM启动程序，请手动关闭后点击\"继续\"，请选择操作",
						u8"继续",
						u8"取消");
					if (ret)
					{
						//iter.KillProcess(iter->th32ProcessID);
						auto hwnd=Alime::ProcessIterator::FindMainHwndByPid(iter->th32ProcessID);
						if(hwnd)
							SendMessage(hwnd, WM_USER + 7, 0, 0);
						qDebug() << "notify user to close app";
						break;//从头检查一次. fix me, or continue
					}
					else
					{
						qDebug() << "user stopped setup";
						return false;
					}				
				}
				++iter;
			}
			if (!findApp)
				return true;
		}
		return true;
	}

private:
	std::wstring matchName_;
};
