
#include "Console.h"

#define ALIME_MSVC
#ifdef COMPILER_GCC
#include <stdlib.h>
#endif

namespace Alime
{
	void Console::CreateConsole()
	{
#ifdef NoConsole
		return;
#endif

		HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (NULL == outHandle)
		{
			AllocConsole();
		}
		else
		{
			FreeConsole();
			AllocConsole();
		}
	}

	void Console::Write(const wchar_t* string, int32_t length)
	{
#ifdef NoConsole
		return;
#endif
#if defined ALIME_MSVC
		HANDLE outHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD fileMode = 0;
		DWORD written = 0;
		if ((GetFileType(outHandle) & FILE_TYPE_CHAR) && GetConsoleMode(outHandle, &fileMode))
		{
			WriteConsole(outHandle, string, (int)length, &written, 0);
		}
		else
		{
			int codePage = GetConsoleOutputCP();
			int charCount = WideCharToMultiByte(codePage, 0, string, -1, 0, 0, 0, 0);
			char* codePageBuffer = new char[charCount];
			memset(codePageBuffer, 0, charCount);
			WideCharToMultiByte(codePage, 0, string, -1, codePageBuffer, charCount, 0, 0);
			WriteFile(outHandle, codePageBuffer, charCount - 1, &written, 0);
			delete[] codePageBuffer;
		}
#elif defined COMPILER_GCC
		wstring s(string, string + length);
		wcout << s << ends;
#endif
}

	void Console::Write(const wchar_t* string)
	{
		Write(string, wcslen(string));
	}

	void Console::Write(const std::wstring& string)
	{
		Write(string.data(), string.length());
	}

	void Console::WriteLine(const std::wstring& string)
	{
		Write(string);
		Write(L"\r\n");
	}

	void Console::WriteLine(const wchar_t* buffer)
	{
		Write(buffer);
		Write(L"\r\n");
	}

	void Console::WriteLine(const wchar_t* buffer, WORD colorType)
	{
		SetColor(colorType);
		Write(buffer);
		Write(L"\r\n");
		SetColor(WHITE);
	}

	std::wstring Console::Read()
	{
#ifdef NoConsole
		return L"";
#endif
#if defined ALIME_MSVC
		std::wstring result;
		DWORD count;
		for (;;)
		{
			wchar_t buffer;
			ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &buffer, 1, &count, 0);
			if (buffer == L'\r')
			{
				ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &buffer, 1, &count, 0);
				break;
			}
			else if (buffer == L'\n')
				break;
			else if (buffer == L' ' || buffer == '\t')
				break;
			else
				result = result + buffer;
		}
		return result;
#elif defined COMPILER_GCC
		wstring s;
		wcin >> s;
		return s.c_str();
#endif
	}

	std::wstring Console::ReadLine()
	{
#ifdef NoConsole
		return L"";
#endif
#if defined ALIME_MSVC
		std::wstring result;
		DWORD count;
		for (;;)
		{
			wchar_t buffer;
			ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &buffer, 1, &count, 0);
			if (buffer == L'\r')
			{
				ReadConsole(GetStdHandle(STD_INPUT_HANDLE), &buffer, 1, &count, 0);
				break;
			}
			else if (buffer == L'\n')
			{
				break;
			}
			else
			{
				result = result + buffer;
			}
		}
		return result;
#elif defined COMPILER_GCC
		wstring s;
		getline(wcin, s, L'\n');
		return s.c_str();
#endif
	}



	void Console::SetColor(bool red, bool green, bool blue, bool light)
	{
#ifdef NoConsole
		return;
#endif
#if defined ALIME_MSVC
		WORD attribute = 0;
		if (red)attribute |= FOREGROUND_RED;
		if (green)attribute |= FOREGROUND_GREEN;
		if (blue)attribute |= FOREGROUND_BLUE;
		if (light)attribute |= FOREGROUND_INTENSITY;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attribute);
		SetConsoleTextAttribute(GetStdHandle(STD_INPUT_HANDLE), attribute);
#elif defined COMPILER_GCC
		int color = (blue ? 1 : 0) * 4 + (green ? 1 : 0) * 2 + (red ? 1 : 0);
		if (light)
			wprintf(L"\x1B[00;3%dm", color);
		else
			wprintf(L"\x1B[01;3%dm", color);
#endif
	}

	void Console::SetColor(WORD colorType)
	{
#ifdef NoConsole
		return;
#endif
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorType);
	}

	void Console::SetTitle(const std::wstring& string)
	{
#ifdef NoConsole
		return;
#endif
#if defined ALIME_MSVC
		SetConsoleTitle(string.data());
#endif
	}

	void Console::Clear()
	{
#ifdef NoConsole
		return;
#endif
#if defined ALIME_MSVC
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		DWORD count = 0;
		FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), L' ', (info.dwSize.X)*(info.dwSize.Y), { 0,0 }, &count);
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
#elif defined COMPILER_GCC
		include <stdlib.h>
			system("cls");
#endif

	}

	void Console::SetWindowPosition(int left, int top)
	{
#ifdef NoConsole
		return;
#endif
#if defined ALIME_MSVC
		//check handle first
		auto hwnd = GetConsoleWindow();
		RECT rc;
		GetWindowRect(hwnd, &rc);
		MoveWindow(hwnd, left, top, rc.right-rc.left, rc.bottom-rc.top, 1);
#elif defined COMPILER_GCC

#endif
	}

	void Console::SetWindowSize(int width, int height)
	{
#ifdef NoConsole
		return;
#endif
#if defined ALIME_MSVC
		auto hwnd = GetConsoleWindow();
		RECT rc;
		GetWindowRect(hwnd, &rc);
		MoveWindow(hwnd, rc.left, rc.top, width, height, 1);
#elif defined COMPILER_GCC

#endif
	}
}

