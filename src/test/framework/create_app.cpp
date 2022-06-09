#include "test/framework/create_app.h"

std::string& GetCurrentAppName() 
{
	static std::string gCurrentAppName;
	return gCurrentAppName;
}

typedef std::map<std::string, std::function<IApp*()>> AppCreatorByName;
AppCreatorByName& RegAppClasses() 
{
	static AppCreatorByName gRegAppClasses;
	return gRegAppClasses;
}
void RegisterApp(const char* name, std::function<IApp* ()> appCls) {
	GetCurrentAppName() = name;
	RegAppClasses()[name] = appCls;
}

IApp* CreateApp(std::string name)
{
	if (name.empty()) name = GetCurrentAppName();
	
	auto entry = RegAppClasses()[name];
	assert(entry);
	IApp* gApp = entry();
	gApp->Create();
	return gApp;
}

LPSTR ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	try
	{
		if (lpwszStrIn != NULL)
		{
			int nInputStrLen = wcslen(lpwszStrIn);
			int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
			pszOut = new char[nOutputStrLen];
			if (pszOut)
			{
				memset(pszOut, 0x00, nOutputStrLen);
				WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
			}
		}
	}
	catch (std::exception e)
	{
	}

	return pszOut;
}

IApp* CreateAppByCommandLineOrCommandFile()
{
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	std::string appName = GetCurrentAppName();
	int caseIndex = -1, caseSecondIndex = -1;
	if (argc > 1) {
		appName = ConvertLPWSTRToLPSTR(argv[1]);
		if (argc > 2) caseIndex = atoi(ConvertLPWSTRToLPSTR(argv[2]));
	}
	else {
		FILE* fd = fopen("test_cmdline.txt", "r");
		if (fd) {
			bool scanf_eof = false;
			do {
				char szAppName[260];
				if (scanf_eof = fscanf(fd, "%s %d %d", szAppName, &caseIndex, &caseSecondIndex) == EOF)
					break;
				if (strlen(szAppName) > 0 && szAppName[0] >= '0' && szAppName[0] <= '9') {
					int appNameIndex = atoi(szAppName) + 1;
					while (appNameIndex--) {
						int tempIndex;
						if (scanf_eof = fscanf(fd, "%s %d", szAppName, &tempIndex) == EOF)
							break;
						if (szAppName[0] >= '0' && szAppName[0] <= '9')
							appNameIndex++;
					}
				}
				appName = szAppName;
			} while (0);
			if (scanf_eof)
				MessageBoxA(NULL, "parse test_cmdline.txt error", "error", MB_OK);
			fclose(fd);
		}
	}

	auto AppDraw = CreateApp(appName);
	if (caseIndex != -1) 
		AppDraw->SetCaseIndex(caseIndex);
	if (caseSecondIndex != -1) 
		AppDraw->SetCaseSecondIndex(caseSecondIndex);

	return AppDraw;
}
