#pragma once
#include "TPredefine.h"
#include "IRenderSystem.h"
#include "TMovable.h"
#include "TInterfaceType.h"
#include "TMaterial.h"
#include "Utility.h"

class AssimpModel;
struct aiNode;
class TApp
{
public:
	TApp();
	~TApp();
	void Create();
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
	virtual void OnInitLight();
	virtual std::string OnCreateRenderSys();
protected:
	XMMATRIX GetWorldTransform();
protected:
	IRenderSystem* mRenderSys = nullptr;
	SDTimer mTimer;
	TMovablePtr mMove;
	XMFLOAT4 mBackgndColor;
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

