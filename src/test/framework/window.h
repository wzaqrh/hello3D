#pragma once
#include <windows.h>

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, const char* name, int width, int height, HWND* pHandle);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LPSTR ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn);