#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <dinput.h>
#include "Utility.h"
#include <functional>

class TApp
{
public:
	TApp();
	~TApp();
public:
	void Attach(HINSTANCE hInstance, HWND hWnd);
	bool Initialize();
	void CleanUp();
	void Render();
	std::string GetName();
protected:
	virtual void OnPreInitDevice() = 0;
	virtual void OnPostInitDevice() = 0;
	virtual void OnRender() = 0;
protected:
	XMMATRIX GetWorldTransform();
protected:
	SDTimer mTimer;
	TMaterialPtr mMaterial;
	TRenderSystem* mRenderSys;
	cbGlobalParam mGlobalParam;
public:
	std::string mName;
};

extern std::string gCurrentAppName;
extern TApp* gApp;
TApp* CreateApp(std::string name);
void RegisterApp(std::string name, std::function<TApp*()> appCls);

template<class T>
struct AppRegister {
	AppRegister(std::string name) {
		gCurrentAppName = name;
		RegisterApp(name, [name]()->TApp* {
			T* app = new T;
			app->mName = name;
			return app;
		});
	}
};

