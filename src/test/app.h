#pragma once
#include "core/mir.h"
#include "core/base/input.h"
#include "core/base/debug.h"
#include "core/predeclare.h"

struct IApp
{
	virtual void Create() = 0;
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd) = 0;
	virtual void Render() = 0;
	virtual void CleanUp() = 0;
	virtual std::string GetName() = 0;
	virtual void SetCaseIndex(int caseIndex) = 0;
};

class App : public IApp
{
public:
	App();
	~App();
public:
	void Create() override;
	bool Initialize(HINSTANCE hInstance, HWND hWnd) override;
	void Render() override;
	void CleanUp() override;
	std::string GetName() override;
	void SetCaseIndex(int caseIndex) override;
protected:
	virtual void OnPreInitDevice() {};
	virtual void OnPostInitDevice() {};
	virtual void OnRender() = 0;
	virtual void OnInitLight();
	virtual void OnInitCamera();
protected:
	Eigen::Matrix4f GetWorldTransform();
protected:
	mir::Mir* mContext = nullptr;
	mir::input::D3DInput* mInput = nullptr;
	mir::debug::Timer* mTimer;
	mir::TransformPtr mTransform;
	float mMoveDefScale, mOriginCameraDistance;
	Eigen::Vector4f mBackgndColor;
	int mCaseIndex = 0;
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

