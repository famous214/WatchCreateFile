#pragma once
#include <tchar.h>
#include <windows.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

DWORD GetModulePath(HMODULE hModule, LPTSTR szBuff, DWORD dwBuffSize);

BOOL EnableDebugPriv();

BOOL InjectDllToProcess(HANDLE hProcess, LPTSTR pszDll);

BOOL CreateProcessWithDll(LPTSTR szExePath, LPTSTR szCmd, PROCESS_INFORMATION* pProcessInfo, LPTSTR pszDll, BOOL bClose = TRUE);