#pragma once
#include "core/mir.h"
#include "core/base/input.h"
#include "core/base/debug.h"
#include "core/predeclare.h"

struct IApp
{
	virtual void Create() = 0;
	virtual void SetCaseIndex(int caseIndex) = 0;
	virtual void SetCaseSecondIndex(int secondIndex) = 0;
	virtual std::string GetName() = 0;
	virtual int MainLoop(HINSTANCE hInstance, HWND hWnd) = 0;
};

class MirManager
{
public:
	void SetMir(mir::Mir* ctx);
	void SetPPU(float ppu);
protected:
	mir::GuiManagerPtr mGuiMng;
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
	void Create() override final;
	void SetCaseIndex(int caseIndex) override final;
	void SetCaseSecondIndex(int secondIndex) override final;
	std::string GetName() override final;
	int MainLoop(HINSTANCE hInstance, HWND hWnd) override final;
private:
	CoTask<bool> InitContext(HINSTANCE hInstance, HWND hWnd);
	CoTask<bool> InitScene();
	CoTask<void> Render();
	void CleanUp();
protected:
	virtual CoTask<bool> OnInitScene() { CoReturn true; };
	virtual void OnInitLight();
	virtual void OnInitCamera();
protected:
	int mCaseIndex = 0, mCaseSecondIndex = 0;
	bool mControlCamera = true;
	mir::Mir* mContext = nullptr;
	mir::TransformPtr mTransform;
private:
	mir::debug::Timer* mTimer = nullptr;
	mir::input::D3DInput* mInput = nullptr;
	Eigen::Vector3f mCameraInitInvLengthForward, mCameraInitLookAt;
public:
	std::string mName;
	HWND mHnd;
};