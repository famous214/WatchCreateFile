////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 头文件
#include <windows.h>
#include "detours.h"
#pragma comment( lib, "detours.lib")
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef HANDLE(WINAPI *CREATEFILEA)(
	LPCSTR lpFileName,                         // file name
	DWORD dwDesiredAccess,                      // access mode
	DWORD dwShareMode,                          // share mode
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
	DWORD dwCreationDisposition,                // how to create
	DWORD dwFlagsAndAttributes,                 // file attributes
	HANDLE hTemplateFile                        // handle to template file
	);

typedef HANDLE(WINAPI *CREATEFILEW)(
	LPCWSTR lpFileName,                         // file name
	DWORD dwDesiredAccess,                      // access mode
	DWORD dwShareMode,                          // share mode
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
	DWORD dwCreationDisposition,                // how to create
	DWORD dwFlagsAndAttributes,                 // file attributes
	HANDLE hTemplateFile                        // handle to template file
	);

typedef BOOL(WINAPI *SETCURRENTDIRECTORYA)(
	LPCSTR lpPathName
	);

typedef BOOL(WINAPI *SETCURRENTDIRECTORYW)(
	LPCWSTR lpPathName
	);

extern HANDLE hLogFile;

CREATEFILEA pfnOrgCreateFileA;
CREATEFILEW pfnOrgCreateFileW;

SETCURRENTDIRECTORYA pfnOrgSetCurrentDirectoryA;
SETCURRENTDIRECTORYW pfnOrgSetCurrentDirectoryW;

HANDLE WINAPI MyCreateFileA(
	LPCSTR lpFileName,                         // file name
	DWORD dwDesiredAccess,                      // access mode
	DWORD dwShareMode,                          // share mode
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
	DWORD dwCreationDisposition,                // how to create
	DWORD dwFlagsAndAttributes,                 // file attributes
	HANDLE hTemplateFile                        // handle to template file
)
{
	//MessageBox(NULL, ObjectAttributes->ObjectName->Buffer, TEXT("Hooked"), MB_OK);
	TCHAR	szPath[MAX_PATH + 1];
	DWORD	dwLenWritten;
	__try
	{
		if (lpFileName)
		{
			int nLen = wsprintfW(szPath, TEXT("CreateFileA::%S\r\n"), lpFileName);
			WriteFile(hLogFile, szPath, nLen * sizeof(TCHAR), &dwLenWritten, NULL);
		}
	}
	__finally
	{
		return pfnOrgCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, \
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
}

HANDLE WINAPI MyCreateFileW(
	LPCWSTR lpFileName,                         // file name
	DWORD dwDesiredAccess,                      // access mode
	DWORD dwShareMode,                          // share mode
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
	DWORD dwCreationDisposition,                // how to create
	DWORD dwFlagsAndAttributes,                 // file attributes
	HANDLE hTemplateFile                        // handle to template file
)
{
	TCHAR	szPath[MAX_PATH + 1];
	DWORD	dwLenWritten;
	__try
	{
		if (lpFileName)
		{
			int nLen = wsprintfW(szPath, TEXT("CreateFileA::%s\r\n"), lpFileName);
			WriteFile(hLogFile, szPath, nLen * sizeof(TCHAR), &dwLenWritten, NULL);
		}
	}
	__finally
	{
		return pfnOrgCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, \
			lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
}

BOOL MySetCurrentDirectoryA(LPCSTR lpPathName)
{
	TCHAR	szPath[MAX_PATH + 1];
	DWORD	dwLenWritten;
	__try
	{
		if (lpPathName)
		{
			int nLen = wsprintfW(szPath, TEXT("MySetCurrentDirectoryA::%S\r\n"), lpPathName);
			WriteFile(hLogFile, szPath, nLen * sizeof(TCHAR), &dwLenWritten, NULL);
		}
	}
	__finally
	{
		return pfnOrgSetCurrentDirectoryA(lpPathName);
	}
}

BOOL MySetCurrentDirectoryW(LPWSTR lpPathName)
{
	TCHAR	szPath[MAX_PATH + 1];
	DWORD	dwLenWritten;
	__try
	{
		if (lpPathName)
		{
			int nLen = wsprintfW(szPath, TEXT("MySetCurrentDirectoryW::%s\r\n"), lpPathName);
			WriteFile(hLogFile, szPath, nLen * sizeof(TCHAR), &dwLenWritten, NULL);
		}
	}
	__finally
	{
		return pfnOrgSetCurrentDirectoryW(lpPathName);
	}
}

BOOL BeginHook()
{
	HMODULE hDll = ::LoadLibrary(TEXT("kernel32.dll"));
	if (NULL == hDll)
	{
		return FALSE;
	}

	pfnOrgCreateFileA = (CREATEFILEA)GetProcAddress(hDll, "CreateFileA");
	pfnOrgCreateFileW = (CREATEFILEW)GetProcAddress(hDll, "CreateFileW");
	pfnOrgSetCurrentDirectoryA = (SETCURRENTDIRECTORYA)GetProcAddress(hDll, "SetCurrentDirectoryA");
	pfnOrgSetCurrentDirectoryW = (SETCURRENTDIRECTORYW)GetProcAddress(hDll, "SetCurrentDirectoryW");

	DetourRestoreAfterWith();                            //恢复原来状态
	DetourTransactionBegin();                            //拦截开始
	DetourUpdateThread(GetCurrentThread());              //刷新当前线程
														 //这里可以连续多次调用DetourAttach，表明HOOK多个函数
	DetourAttach((void **)&pfnOrgCreateFileA, MyCreateFileA);//实现函数拦截
	DetourAttach((void **)&pfnOrgCreateFileW, MyCreateFileW);//实现函数拦截
	DetourAttach((void **)&pfnOrgSetCurrentDirectoryA, MySetCurrentDirectoryA);//实现函数拦截
	DetourAttach((void **)&pfnOrgSetCurrentDirectoryW, MySetCurrentDirectoryW);//实现函数拦截
	DetourTransactionCommit();                           //拦截生效

	MessageBox(NULL, TEXT("Hook success!"), TEXT("success"), MB_ICONQUESTION | MB_OK);
	return TRUE;
}

void EndHook()
{
	DetourTransactionBegin();                            //拦截开始
	DetourUpdateThread(GetCurrentThread());              //刷新当前线程
														 //这里可以连续多次调用DetourAttach，表明撤销HOOK多个函数
	DetourDetach((void **)&pfnOrgCreateFileA, MyCreateFileA);
	DetourDetach((void **)&pfnOrgCreateFileW, MyCreateFileW);
	DetourDetach((void **)&pfnOrgSetCurrentDirectoryA, MySetCurrentDirectoryA);
	DetourDetach((void **)&pfnOrgSetCurrentDirectoryW, MySetCurrentDirectoryW);
	DetourTransactionCommit();                           //拦截生效
	MessageBox(NULL, TEXT("Unhook success!"), TEXT("success"), MB_ICONQUESTION | MB_OK);
	return;
}