#pragma once

#include <string>
#include <algorithm>
#include <regex>

#include "Alime/processIterator.h"
#include "AppUtility.h"

class ProcessManager
{
public:
	ProcessManager() = default;

	void SetMatchReg(const std::wstring& name)
	{
		matchName_ = name;
	}

	bool ShutDownExistingApp()
	{
		using Alime::ProcessIterator;

		bool customNotified = false;
		bool stop = false;
		ProcessIterator iter;
		while (iter.hasNext()&& !stop)
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
					auto ret = ShowQuestionBox(u8"��⵽PKPM��س������ڹ���",
						u8"���²�����Ҫ�ر����е�PKPM��س��򣬵��\"����\"��ǿ�ƹرճ�����ѡ�����",
						u8"����",
						u8"ȡ��");
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


	//���ز����Ƿ��û��ж�
	//fix me�� �ظ��Ĵ���
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
					auto ret = ShowQuestionBox(u8"��⵽PKPM�����������ڹ���",
						u8"���²�����Ҫ�ر����е�PKPM��س��򣬵��\"����\"��ǿ�ƹرճ�����ѡ�����",
						u8"����",
						u8"ȡ��");
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
