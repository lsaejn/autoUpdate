#pragma once
#include <string>

#include <windows.h>
#include <tlhelp32.h>
#include <vector>

#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

namespace Alime
{

    class ProcessIterator
    {
    private:
        HANDLE hProcessSnap;
        PROCESSENTRY32 pe32;
        bool active;
    public:
        ProcessIterator()
            : active(false),
            pe32(),
            hProcessSnap(NULL)
        {
            hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hProcessSnap == INVALID_HANDLE_VALUE)
                return;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            if (Process32First(hProcessSnap, &pe32))
                active = true;
        }

        ~ProcessIterator()
        {
            if (hProcessSnap != INVALID_HANDLE_VALUE)
                CloseHandle(hProcessSnap);
        }

        void operator++()
        {
            active = (Process32Next(hProcessSnap, &pe32) == TRUE);
        }

        bool hasNext()
        {
            return active;
        }

        PROCESSENTRY32* operator->()
        {
            return &pe32;
        }

        PROCESSENTRY32* operator*()
        {
            return &pe32;
        }

        std::wstring getPath()
        {
            return pe32.szExeFile;
        }

        static std::wstring GetAppFullPathByPid(DWORD processID)
        {
            //HMODULE hMods[1024] = { 0 };
            //DWORD cbNeeded = 0;
            //unsigned int i = 0;
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE, processID);
            if(!hProcess)
                return {};
            wchar_t buffer[1024] = { 0 };
            DWORD size=sizeof(buffer)/sizeof(buffer[0]);
            BOOL ret=QueryFullProcessImageNameW(hProcess, 0, buffer, &size);
            if (ret)
                return buffer;
            return {};
        }

        static std::wstring GetAppNameByPid(DWORD processID)
        {
            std::wstring result = GetAppFullPathByPid(processID);
            if (result.empty())
                return result;
            else
            {
                size_t index=result.find_last_of(L"/\\");
                if (index == std::wstring::npos)
                    return result;
                return result.substr(0, index);
            }
        }

        std::vector<HWND> static FindHwndByPid(DWORD dwProcessId)
        {
            std::vector<HWND> re;
            HWND hWnd = GetTopWindow(NULL);
            while (hWnd)
            {
                DWORD id;
                GetWindowThreadProcessId(hWnd, &id);
                if (id == dwProcessId)
                {
                    re.push_back(hWnd);
                    hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
                }
                else
                    hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
            }
            return re;
        }

        //我们只能假设这个程序开启一个主窗口
        HWND static FindMainHwndByPid(DWORD dwProcessId)
        {
            HWND hWnd = GetTopWindow(NULL);
            while (hWnd)
            {
                DWORD id;
                GetWindowThreadProcessId(hWnd, &id);
                if (id == dwProcessId
                    && IsWindow(hWnd)
                    && IsWindowVisible(hWnd)
                    && ::GetParent(hWnd)==NULL)
                {
                    HWND h=hWnd;
                    return h;
                }
                else
                    hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
            }
            return NULL;
        }

        static std::vector<DWORD> GetPidsOfChildProcess(DWORD parentPid)
        {
            std::vector<DWORD> pidsOfChild;
            HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hProcessSnap == INVALID_HANDLE_VALUE)
            {
                return pidsOfChild;
            }
            Win32HandleGuard guard(hProcessSnap);
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(pe);
            BOOL bProcess = Process32First(hProcessSnap, &pe);
            DWORD pidOfChild = 0;
            while (bProcess)
            {
                if (pe.th32ParentProcessID == parentPid)	
                {
                    pidOfChild = pe.th32ProcessID;
                    pidsOfChild.push_back(pidOfChild);
                }
                bProcess = Process32Next(hProcessSnap, &pe);
            }
            return pidsOfChild;
        }

        static HANDLE GetHandleFromPid(DWORD pid)
        {
            HANDLE handle = INVALID_HANDLE_VALUE;
            handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (INVALID_HANDLE_VALUE == handle)
                handle = OpenProcess(PROCESS_VM_READ, FALSE, pid);
            return handle;
        }

        static void KillProcess(DWORD pid)
        {
            ::TerminateProcess(GetHandleFromPid(pid), 1);
        }

        class Win32HandleGuard
        {
        public:
            Win32HandleGuard(HANDLE h)
                :h_(h)
            {
            }

            ~Win32HandleGuard()
            {
                if (INVALID_HANDLE_VALUE != h_)
                    ::CloseHandle(h_);
            }
        private:
            HANDLE h_;
        };

    };

}