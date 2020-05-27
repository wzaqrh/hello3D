#pragma once
#include "IRenderSystem.h"
#include "TInterfaceTypePred.h"
#include "TMaterialPred.h"

class TAssimpModel;
struct aiNode;

struct IApp
{
	virtual void Create() = 0;
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd) = 0;
	virtual void Render() = 0;
	virtual void CleanUp() = 0;
	virtual std::string GetName() = 0;
};

typedef std::shared_ptr<struct TMovable> TMovablePtr;
class TApp : public IApp
{
public:
	TApp();
	~TApp();
public:
	virtual void Create();
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd);
	virtual void Render();
	virtual void CleanUp();
	virtual std::string GetName();
protected:
	virtual void OnPreInitDevice() {};
	virtual void OnPostInitDevice() {};
	virtual void OnRender() = 0;
	virtual void OnInitLight();
protected:
	XMMATRIX GetWorldTransform();
protected:
	IRenderSystem* mRenderSys = nullptr;
	class TD3DInput* mInput = nullptr;
	class SDTimer* mTimer;
	TMovablePtr mMove;
	XMFLOAT4 mBackgndColor;
public:
	std::string mName;
	HWND mHnd;
};


IApp* CreateApp(std::string name);
void RegisterApp(std::string name, std::function<IApp*()> appCls);

extern std::string gCurrentAppName;
template<class T> struct AppRegister {
	AppRegister(std::string name) {
		gCurrentAppName = name;
		RegisterApp(name, [name]()->IApp* {
			T* app = new T;
			app->mName = name;
			return app;
		});
	}
};

