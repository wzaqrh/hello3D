// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

#if _MSC_VER > 1800
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "d3d9.lib")
#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#ifdef _DEBUG
#pragma comment(lib, "d3dx11d.lib")
#pragma comment(lib, "d3dx10d.lib")
#else
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#endif // DEBUG
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "assimp-vc140-mt.lib")
#pragma comment(lib, "freetype.lib")
#ifdef _DEBUG
#pragma comment(lib, "DevILd.lib")
#else
#pragma comment(lib, "DevIL.lib")
#endif

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

