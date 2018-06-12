#include "Hook.h"
#include "..\\WatchCreateFile\\HelperFunc.h"
#include <tchar.h>


HANDLE	hLogFile;


BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	static TCHAR	szTime[0x50];
	static TCHAR	szPath[MAX_PATH + 0x20];
	SYSTEMTIME	st;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		GetModulePath(hModule, szPath, ARRAY_SIZE(szPath));
		GetSystemTime(&st);
		wsprintf(szTime, TEXT("%u%u%u%u%u-CreateFileLog.log"), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		_tcscat(szPath, szTime);
		hLogFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hLogFile)
		{
			MessageBox(NULL, TEXT("Error while creating log file"), TEXT("Error"), MB_ICONERROR | MB_OK);
			return FALSE;
		}
		BeginHook();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		EndHook();
		MessageBox(NULL, TEXT("End Hook"), TEXT("Haha"), MB_ICONQUESTION | MB_OK);
		CloseHandle(hLogFile);
	}

	return TRUE;
}