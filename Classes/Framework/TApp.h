#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <dinput.h>
#include <functional>
#include "Utility.h"
#include "TBaseTypes.h"

class AssimpModel;
struct aiNode;
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
	virtual void OnPreInitDevice() {};
	virtual void OnPostInitDevice() {};
	virtual void OnRender() = 0;
protected:
	XMMATRIX GetWorldTransform();
protected:
	SDTimer mTimer;
	TRenderSystem* mRenderSys;
	//TMaterialPtr mMaterial;
	float mScale;
	XMFLOAT3 mPosition;
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

