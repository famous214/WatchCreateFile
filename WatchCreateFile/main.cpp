#include "HelperFunc.h"
#include "CppIOHeader.h"

void Init()
{
	//EnableDebugPriv();

	wcin.imbue(locale("chs"));
	wcout.imbue(locale("chs"));
}

int main()
{
	Init();

	// 构造待注入Dll的全路径（用当前Exe路径加Dll名得到）
	TCHAR szPath[MAX_PATH + 0x20];
	DWORD dwTest = GetModulePath(NULL, szPath, ARRAY_SIZE(szPath));
	_tcscat(szPath, TEXT("LogFileAction.dll"));

	// 接收目标程序全路径（拖拽方式）
	tstring strExePath;
	tcout << TEXT("Please input the file path(drag an executable file to this console):");
	tcin >> strExePath;
	tstring szCmdApp(strExePath, strExePath.rfind(TEXT('\\')) + 1);

	// 接收目标程序运行所需参数
	tcout << TEXT("Have command-line argument?(Y/y for yes, others for no):");
	TCHAR answer;
	tcin >> answer;
	if (answer == TEXT('Y') || answer == TEXT('y'))
	{
		tstring strCmdLine;
		// 忽略掉之前的回车换行符
		cin.ignore(INT_MAX, '\n');
		tcout << TEXT("Please input the command-lines:");
		// 读取一行的内容
		getline(tcin, strCmdLine);
		szCmdApp = szCmdApp + TEXT(" ") + strCmdLine;
	}
	
	
	// 悬挂启动目标进程并注入Dll
	if (!CreateProcessWithDll((LPTSTR)strExePath.c_str(), (LPTSTR)szCmdApp.c_str(), NULL, szPath))
	{
		tcout << TEXT("创建目标进程出现错误，请重新执行本程序") << endl;
	}
	return 0; 
}