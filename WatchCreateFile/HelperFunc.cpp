#include "HelperFunc.h"

//************************************
// @Method:    	GetCurrentPath
// @Desc:		��ȡָ��ģ�����ڵľ���·��
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
// @Desc:		�������̵ĵ���Ȩ��
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
// @Desc:		��Ŀ��Dllע�뵽ָ��������
// @Access:    	public 
// @Parameter: 	HANDLE hProcess	���̾��
// @Parameter: 	LPTSTR pszDll	Dll·��
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
		// ��Ŀ������з���ռ�洢�ַ���
		pszDllPath = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (NULL == pszDllPath)
		{
			__leave;
		}

		// ��Ŀ������ѷ���Ŀռ���д���ַ���
		bWriteProcessMem = WriteProcessMemory(hProcess, pszDllPath, (LPVOID)pszDll, (_tcslen(pszDll) + 1) * sizeof(TCHAR), NULL);
		if (!bWriteProcessMem)
		{
			__leave;
		}

		// ��LoadLibrary��Ϊ��ַ��Dll����Ϊ��������Ŀ�����������Զ�߳�
		hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, pszDllPath, CREATE_SUSPENDED, NULL);
		if (NULL == hThread)
		{
			__leave;
		}

		// �ָ�Զ�߳�ִ��
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
// @Desc:		�ڴ������̵�ͬʱע��һ��Dll��Ŀ�������
// @Access:    	public 
// @Parameter: 	LPTSTR szExePath	����·����ͬCreateProcess�ĵ�һ������
// @Parameter: 	LPTSTR szCmd	���������У�ͬCreateProcess�ĵڶ�������
// @Parameter: 	PROCESS_INFORMATION * pProcessInfo	��ѡ�����ڽ��ս��̺����߳���Ϣ�Ľṹ����ʹ�ÿɴ�NULL
// @Parameter: 	LPTSTR pszDll ��ע���Dll·��
// @Parameter: 	BOOL bClose	�Ƿ��ɱ������رմ����Ľ��̡��̵߳ľ��
// @Returns:   	BOOL
// @Author:		VirusLee
// @Version(s):	1.0  2018/06/12 11:10:44	��һ�澭������
// @Version(s):	1.1  2018/06/12 15:33:44	�ڶ���Ľ���CreateProcess�Ĳ�����szCmdLine������Ҫ����App��
// @Version(s):	1.1  2018/06/12 16:22:51	���������VC��SEH�ع�����������ǿ��׳�ԣ�������Դ�ͷŴ���
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

		// �����ҷ�ʽ��������
		bCreateProcessRet = CreateProcess(szExePath, szCmd, NULL, NULL, FALSE, \
			CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
		if (!bCreateProcessRet)
		{
			__leave;
		}

		// ��Ŀ�����ע��Dll
		bInjectDll = InjectDllToProcess(pi.hProcess, pszDll);
		if (!bInjectDll)
		{
			__leave;
		}

		// �ָ�Ŀ��������̵߳�ִ��
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
