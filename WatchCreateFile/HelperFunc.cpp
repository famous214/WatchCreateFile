#include "HelperFunc.h"

//************************************
// @Method:    	GetCurrentPath
// @Desc:		获取指定模块所在的绝对路径
// @Access:    	public 
// @Parameter: 	LPTSTR szBuff
// @Parameter: 	DWORD dwBuffSize
// @Returns:   	DWORD
// @Author:		VirusLee
// @Version(s):	1.0  2018/06/11 23:55:17
//************************************
DWORD GetModulePath(HMODULE hModule, LPTSTR szBuff, DWORD dwBuffSize)
{
	if (GetModuleFileName(hModule, szBuff, MAX_PATH) <= 0)
	{
		return 0;
	}
	LPTSTR pEndToken = _tcsrchr(szBuff, TEXT('\\'));
	*(pEndToken + 1) = NULL;
	return pEndToken - szBuff;
}

//************************************
// @Method:    	EnableDebugPriv
// @Desc:		开启进程的调试权限
// @Access:    	public 
// @Returns:   	BOOL
// @Author:		VirusLee
// @Version(s):	1.0  2018/06/12 10:30:06
//************************************
BOOL EnableDebugPriv()
{
	HANDLE   hToken;
	LUID   sedebugnameValue;
	TOKEN_PRIVILEGES   tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return   FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		return   FALSE;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		return   FALSE;
	}
	CloseHandle(hToken);
	return TRUE;
}

//************************************
// @Method:    	InjectDllToProcess
// @Desc:		将目标Dll注入到指定进程中
// @Access:    	public 
// @Parameter: 	HANDLE hProcess	进程句柄
// @Parameter: 	LPTSTR pszDll	Dll路径
// @Returns:   	BOOL
// @Author:		VirusLee
// @Version(s):	1.0  2018/06/12 16:49:50
//************************************
BOOL InjectDllToProcess(HANDLE hProcess, LPTSTR pszDll)
{
	LPVOID		pszDllPath = NULL;
	BOOL		bWriteProcessMem = FALSE;
	HANDLE		hThread = NULL;
	BOOL		bStatus = FALSE;
	__try
	{
		// 在目标进程中分配空间存储字符串
		pszDllPath = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (NULL == pszDllPath)
		{
			__leave;
		}

		// 向目标进程已分配的空间中写入字符串
		bWriteProcessMem = WriteProcessMemory(hProcess, pszDllPath, (LPVOID)pszDll, (_tcslen(pszDll) + 1) * sizeof(TCHAR), NULL);
		if (!bWriteProcessMem)
		{
			__leave;
		}

		// 将LoadLibrary作为地址，Dll名作为参数，在目标进程中启动远线程
		hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, pszDllPath, CREATE_SUSPENDED, NULL);
		if (NULL == hThread)
		{
			__leave;
		}

		// 恢复远线程执行
		ResumeThread(hThread);
		bStatus = TRUE;
	}
	__finally
	{
		if (hThread)
		{
			CloseHandle(hThread);
		}
		if (pszDllPath)
		{
			VirtualFreeEx(hProcess, pszDllPath, (_tcslen(pszDll) + 1) * sizeof(TCHAR), MEM_RELEASE);
		}
		return bStatus;
	}
}

//************************************
// @Method:    	CreateProcessWithDll
// @Desc:		在创建进程的同时注入一个Dll到目标进程中
// @Access:    	public 
// @Parameter: 	LPTSTR szExePath	进程路径，同CreateProcess的第一个参数
// @Parameter: 	LPTSTR szCmd	进程命令行，同CreateProcess的第二个参数
// @Parameter: 	PROCESS_INFORMATION * pProcessInfo	可选：用于接收进程和主线程信息的结构，不使用可传NULL
// @Parameter: 	LPTSTR pszDll 待注入的Dll路径
// @Parameter: 	BOOL bClose	是否由本函数关闭创建的进程、线程的句柄
// @Returns:   	BOOL
// @Author:		VirusLee
// @Version(s):	1.0  2018/06/12 11:10:44	第一版经过测试
// @Version(s):	1.1  2018/06/12 15:33:44	第二版改进了CreateProcess的参数，szCmdLine首先需要传递App名
// @Version(s):	1.1  2018/06/12 16:22:51	第三版采用VC的SEH重构本函数，增强健壮性，加入资源释放代码
//************************************
BOOL CreateProcessWithDll(LPTSTR szExePath, LPTSTR szCmd, PROCESS_INFORMATION* pProcessInfo, LPTSTR pszDll, BOOL bClose)
{
	BOOL		bCreateProcessRet = FALSE;
	BOOL		bInjectDll = FALSE;
	BOOL		bStatus = FALSE;
	PROCESS_INFORMATION pi;
	RtlZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	__try
	{
		STARTUPINFO si;
		RtlZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = TRUE;

		// 以悬挂方式创建进程
		bCreateProcessRet = CreateProcess(szExePath, szCmd, NULL, NULL, FALSE, \
			CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
		if (!bCreateProcessRet)
		{
			__leave;
		}

		// 向目标进程注入Dll
		bInjectDll = InjectDllToProcess(pi.hProcess, pszDll);
		if (!bInjectDll)
		{
			__leave;
		}

		// 恢复目标进程主线程的执行
		ResumeThread(pi.hThread);

		if (pProcessInfo)
		{
			*pProcessInfo = pi;
		}
		bStatus = TRUE;
	}
	__finally
	{
		if (bClose)
		{
			if (pi.hThread)
			{
				CloseHandle(pi.hThread);
			}
			if (pi.hProcess)
			{
				CloseHandle(pi.hProcess);
			}
		}
		return bStatus;
	}
}
