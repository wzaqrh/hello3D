#include <windows.h>
#include "test/framework/app.h"
#include "test/framework/create_app.h"
#include "test/framework/window.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	IApp* AppDraw = CreateAppByCommandLineOrCommandFile();

	int WINDOW_WIDTH, WINDOW_HEIGHT;
	WINDOW_WIDTH = 1200;
	WINDOW_HEIGHT = 800;

	HWND handle;
	if (FAILED(InitWindow(hInstance, nCmdShow, AppDraw->GetName().c_str(), WINDOW_WIDTH, WINDOW_HEIGHT, &handle)))
		return 0;

	return AppDraw->MainLoop(hInstance, handle);
}