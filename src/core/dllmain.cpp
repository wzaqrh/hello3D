// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include "core/mir_config.h"

#pragma comment(lib, "dinput8.lib")
#ifdef _DEBUG
#pragma comment(lib, "d3dx11d.lib")
#else
#pragma comment(lib, "d3dx11.lib")
#endif
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "Opengl32.lib")
#pragma comment(lib, "glfw3.lib")

#ifdef _DEBUG
#pragma comment(lib, "cppcorod.lib")
#pragma comment(lib, "FreeImagePlusd.lib")
#else
#pragma comment(lib, "cppcoro.lib")
#pragma comment(lib, "FreeImagePlus.lib")
#endif
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "assimp.lib")

#if _MSC_VER > 1800
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif
#if defined MIR_TIME_DEBUG
#pragma comment(lib, "winmm.lib")
#endif

#if defined _WINDLL
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif