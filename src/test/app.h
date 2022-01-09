#pragma once
#include "core/mir.h"
#include "core/base/input.h"
#include "core/base/debug.h"
#include "core/predeclare.h"

#define C_WINDOW_WIDTH (2884/2)
#define C_WINDOW_HEIGHT (1724/2)

struct IApp
{
	virtual void Create() = 0;
	virtual bool Initialize(HINSTANCE hInstance, HWND hWnd) = 0;
	virtual void Render() = 0;
	virtual void CleanUp() = 0;
	virtual std::string GetName() = 0;
	virtual void SetCaseIndex(int caseIndex) = 0;
	virtual void SetCaseSecondIndex(int secondIndex) = 0;
};

class MirManager
{
public:
	void SetMir(mir::Mir* ctx);
	void SetPPU(float ppu);
protected:
	mir::SceneManagerPtr mScneMng;
	mir::RenderableFactoryPtr mRendFac;
	mir::ResourceManagerPtr mResMng;
	Eigen::Vector3f mWinCenter, mHalfSize, mCamWinHSize;
};

class App : public IApp, public MirManager
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
	App();
	~App();
public:
	void Create() override;
	bool Initialize(HINSTANCE hInstance, HWND hWnd) override;
	void Render() override;
	void CleanUp() override;
	std::string GetName() override;
	void SetCaseIndex(int caseIndex) override;
	void SetCaseSecondIndex(int secondIndex) override;
protected:
	virtual void OnPreInitDevice() {};
	virtual void OnPostInitDevice() {};
	virtual void OnRender() = 0;
	virtual void OnInitLight();
	virtual void OnInitCamera();
protected:
	Eigen::Matrix4f GetWorldTransform();
protected:
	Eigen::Vector4f mBackgndColor;
	int mCaseIndex = 0, mCaseSecondIndex = 0;
	mir::Mir* mContext = nullptr;
	mir::TransformPtr mTransform;
	bool mControlCamera = true;
	mir::debug::Timer* mTimer = nullptr;
private:
	mir::input::D3DInput* mInput = nullptr;
	Eigen::Vector3f mCameraInitInvLengthForward, mCameraInitLookAt;
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

