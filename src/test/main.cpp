//--------------------------------------------------------------------------------------
// File: Tutorial06.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include "test/framework/app.h"
#include "test/framework/create_app.h"
#include "test/framework/window.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	IApp* AppDraw = CreateAppByCommandLineOrCommandFile();

	int WINDOW_WIDTH, WINDOW_HEIGHT;
	if (AppDraw->GetName() == "test_gltf") {
		WINDOW_WIDTH = 1520;
		WINDOW_HEIGHT = 937;
	}
	else if (AppDraw->GetName() == "test_imgui") {
		WINDOW_WIDTH = 1200;
		WINDOW_HEIGHT = 800;
	}
	else {
		WINDOW_WIDTH = 800;
		WINDOW_HEIGHT = 600;
	}

	WINDOW_WIDTH = 800;
	WINDOW_HEIGHT = 600;

	HWND handle;
	if (FAILED(InitWindow(hInstance, nCmdShow, AppDraw->GetName().c_str(), WINDOW_WIDTH, WINDOW_HEIGHT, &handle)))
		return 0;

	{
		SetWindowPos(handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE);
		POINT topleft = { 0, 0 };
		ClientToScreen(handle, &topleft);

		if (AppDraw->GetName() == "test_gltf") {
			SetWindowPos(handle, HWND_NOTOPMOST, 0 - topleft.x, 103 - topleft.y, 0, 0, SWP_NOSIZE);
		}
		else {
			SetWindowPos(handle, HWND_NOTOPMOST, 471 - topleft.x, 32 + 94 - topleft.y, 0, 0, SWP_NOSIZE);
		}
	#if defined _DEBUG
		/*RECT rect;
		GetClientRect(handle, &rect);
		BOOST_ASSERT(rect.right - rect.left == WINDOW_WIDTH);
		BOOST_ASSERT(rect.bottom - rect.top == WINDOW_HEIGHT);*/
	#endif
	}

	return AppDraw->MainLoop(hInstance, handle);
}