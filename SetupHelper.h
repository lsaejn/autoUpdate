#pragma once

#include <string>
#include <algorithm>
#include <regex>

#include "Alime/processIterator.h"
#include "AppUtility.h"

/*
fix me, ���������ظ���û��ʱ����д
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
					auto ret = ShowQuestionBox(u8"��⵽PKPM��س������ڹ���",
						u8"���²�����Ҫ�ر����е�PKPM��س�����رճ��������������ѡ�����",
						u8"����",
						u8"ȡ��");
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
						auto ret = ShowQuestionBox(u8"��⵽PKPM��س������ڹ���",
							u8"���²�����Ҫ�ر����е�PKPM��س�����رճ��������������ѡ�����",
							u8"����",
							u8"ȡ��");
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


	//���ز����Ƿ��û��ж�, 
	//true ��֤�û�����ر���س���
	//false �û�ȡ��
	//fix me�� �ظ��Ĵ���
	bool ShutDownFuzzyMatchApp()
	{
		using Alime::ProcessIterator;

		bool customNotified = false;
		bool findApp = false;
		//
		
		while (1)//�û����ܿ���������֮�����������ر�
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
					auto ret = ShowQuestionBox(u8"��⵽PKPM�����������ڹ���",
						u8"���²�����Ҫ�ر����е�PKPM�����������ֶ��رպ���\"����\"����ѡ�����",
						u8"����",
						u8"ȡ��");
					if (ret)
					{
						//iter.KillProcess(iter->th32ProcessID);
						auto hwnd=Alime::ProcessIterator::FindMainHwndByPid(iter->th32ProcessID);
						if(hwnd)
							SendMessage(hwnd, WM_USER + 7, 0, 0);
						qDebug() << "notify user to close app";
						break;//��ͷ���һ��. fix me, or continue
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
