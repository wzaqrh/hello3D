//--------------------------------------------------------------------------------------
// File: Tutorial06.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include "core/rendersys/render_system.h"
#include "test/app.h"

#pragma comment(lib, "mir.lib")
#ifdef _DEBUG
#pragma comment(lib, "cppcorod.lib")
#else
#pragma comment(lib, "cppcoro.lib")
#endif

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, const char* name, HWND* pHandle);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char* ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int argc = 0; 
	LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);

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

	HWND handle;
	if (FAILED(InitWindow(hInstance, nCmdShow, AppDraw->GetName().c_str(), &handle)))
		return 0;

	SetWindowPos(handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE);
	POINT topleft = {0, 0};
	ClientToScreen(handle, &topleft);
	SetWindowPos(handle, HWND_NOTOPMOST, 0 - topleft.x, 103 - topleft.y, 0, 0, SWP_NOSIZE);

#if defined _DEBUG
	RECT rect;
	GetClientRect(handle, &rect);
	BOOST_ASSERT(rect.bottom - rect.top == C_WINDOW_HEIGHT);
	BOOST_ASSERT(rect.right - rect.left == C_WINDOW_WIDTH);
#endif
	return AppDraw->MainLoop(hInstance, handle);
}

char* ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn)
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
	{}

	return pszOut;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, const char* name, HWND* pHandle)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)0);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)0);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	RECT rc = { 0, 0, C_WINDOW_WIDTH, C_WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	auto HWnd = CreateWindowA("TutorialWindowClass", name, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);

	if (!HWnd)
		return E_FAIL;

	*pHandle = HWnd;

	ShowWindow(HWnd, nCmdShow);
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}