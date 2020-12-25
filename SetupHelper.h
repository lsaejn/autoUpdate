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
	//true �û����ȷ�����������������汻�ر�
	//false �û�ȡ��
	//fix me�� �ظ��Ĵ���
	bool ShutDownFuzzyMatchApp()
	{
		//fix me,ȥ��
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
				auto ret = ShowQuestionBox(u8"��⵽PKPM�����������ڹ���",
					u8"���²�����Ҫ�ر����е�PKPM�����������ֶ��رպ���\"����\"����ѡ�����",
					u8"����",
					u8"ȡ��");
				if (ret)
				{
					//�����¡��ٰ��û���һ��
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

	//��һ���ܹص�
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
