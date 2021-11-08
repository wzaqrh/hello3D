#pragma once
#include "core/mir.h"
#include "core/rendersys/predeclare.h"

namespace mir {
	struct aiNode;
	class AssimpModel;
	class D3DInput;
	class Timer;
	struct Movable;
	typedef std::shared_ptr<struct Movable> MovablePtr;
}

struct IApp
{
	virtual void Create() = 0;
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd) = 0;
	virtual void Render() = 0;
	virtual void CleanUp() = 0;
	virtual std::string GetName() = 0;
};

typedef std::shared_ptr<mir::Movable> TMovablePtr;
class App : public IApp
{
public:
	App();
	~App();
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
	mir::Mir* mContext = nullptr;
	class mir::D3DInput* mInput = nullptr;
	class mir::Timer* mTimer;
	mir::MovablePtr mMove;
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

