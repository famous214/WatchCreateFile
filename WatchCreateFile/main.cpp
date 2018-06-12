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

	// �����ע��Dll��ȫ·�����õ�ǰExe·����Dll���õ���
	TCHAR szPath[MAX_PATH + 0x20];
	DWORD dwTest = GetModulePath(NULL, szPath, ARRAY_SIZE(szPath));
	_tcscat(szPath, TEXT("LogFileAction.dll"));

	// ����Ŀ�����ȫ·������ק��ʽ��
	tstring strExePath;
	tcout << TEXT("Please input the file path(drag an executable file to this console):");
	tcin >> strExePath;
	tstring szCmdApp(strExePath, strExePath.rfind(TEXT('\\')) + 1);

	// ����Ŀ����������������
	tcout << TEXT("Have command-line argument?(Y/y for yes, others for no):");
	TCHAR answer;
	tcin >> answer;
	if (answer == TEXT('Y') || answer == TEXT('y'))
	{
		tstring strCmdLine;
		// ���Ե�֮ǰ�Ļس����з�
		cin.ignore(INT_MAX, '\n');
		tcout << TEXT("Please input the command-lines:");
		// ��ȡһ�е�����
		getline(tcin, strCmdLine);
		szCmdApp = szCmdApp + TEXT(" ") + strCmdLine;
	}
	
	
	// ��������Ŀ����̲�ע��Dll
	if (!CreateProcessWithDll((LPTSTR)strExePath.c_str(), (LPTSTR)szCmdApp.c_str(), NULL, szPath))
	{
		tcout << TEXT("����Ŀ����̳��ִ���������ִ�б�����") << endl;
	}
	return 0; 
}