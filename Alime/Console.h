#pragma once
#include <stdint.h>
#include <string>
#include "windows.h"
namespace Alime
{
	class Console
	{
	public:

		static const WORD RED = FOREGROUND_RED;
		static const WORD GREEN = FOREGROUND_GREEN;
		static const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
		static const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		static const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;

		static void CreateConsole();
		static void Write(const wchar_t* str, int32_t  length);
		static void Write(const wchar_t* str);
		static void Write(const std::wstring& str);
		static void WriteLine(const wchar_t* buffer);
		static void WriteLine(const wchar_t* buffer, WORD colorType);
		static void WriteLine(const std::wstring& string);
		static std::wstring Read();
		static std::wstring ReadLine();
		static void SetColor(bool red, bool green, bool blue, bool light);
		static void SetColor(WORD colorType);
		static void SetTitle(const std::wstring& string);
		static void Clear();
		static void SetWindowPosition(int left, int top);
		static void SetWindowSize(int width, int height);
	};
}