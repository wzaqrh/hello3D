#pragma once
#include "core/mir.h"
#include "core/predeclare.h"

namespace mir 
{
	struct aiNode;
	class AssimpModel;
	class D3DInput;
	class Timer;
}

struct IApp
{
	virtual void Create() = 0;
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd) = 0;
	virtual void Render() = 0;
	virtual void CleanUp() = 0;
	virtual std::string GetName() = 0;
};

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
	Eigen::Matrix4f GetWorldTransform();
protected:
	mir::Mir* mContext = nullptr;
	class mir::D3DInput* mInput = nullptr;
	class mir::Timer* mTimer;
	mir::TransformPtr mTransform;
	float mMoveDefScale;
	Eigen::Vector4f mBackgndColor;
public:
	std::string mName;
	HWND mHnd;
};


IApp* CreateApp(std::string name);
void RegisterApp(const char* name, std::function<IApp*()> appCls);
std::string& GetCurrentAppName();

template<class T> struct AppRegister {
	AppRegister(const char* name) {
		RegisterApp(name, [name]()->IApp* {
			T* app = new T;
			app->mName = name;
			return app;
		});
	}
};

