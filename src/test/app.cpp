#include <boost/algorithm/clamp.hpp>
#include <boost/filesystem.hpp>
#include "core/base/transform.h"
#include "core/base/input.h"
#include "core/rendersys/render_system.h"
#include "core/scene/scene_manager.h"
#include "test/app.h"
#include "test/test_case.h"

using namespace mir;

//#define AppLaunchMode __LaunchSync__
#define AppLaunchMode __LaunchAsync__

App::App()
{
	mCameraInitInvLengthForward = Eigen::Vector3f::Zero();
	mBackgndColor = Eigen::Vector4f(0.1f, 0.1f, 0.1f, 0.0f);
	mTransform = std::make_shared<mir::Transform>();
	mContext = new mir::Mir(AppLaunchMode);
}
App::~App()
{
	delete mContext;
}

void App::Create()
{}
bool App::Initialize(HINSTANCE hInstance, HWND hWnd)
{
	mHnd = hWnd;

	OnPreInitDevice();
	if (!mContext->Initialize(mHnd)) {
		mContext->Dispose();
		return false;
	}
	SetMir(mContext);
	OnInitCamera();
	OnInitLight();
	OnPostInitDevice();

	mInput = new mir::input::D3DInput(hInstance, hWnd, mContext->RenderSys()->WinSize().x(), mContext->RenderSys()->WinSize().y());
	mTimer = new mir::debug::Timer;
	return true;
}
void App::OnInitCamera()
{
	mContext->SceneMng()->AddPerspectiveCamera(test1::cam::Eye(mWinCenter), test1::cam::NearFarFov());
}
void App::OnInitLight()
{
	mContext->SceneMng()->AddPointLight();
}
void App::CleanUp()
{
	mContext->Dispose();
}

void App::Render()
{
	auto renderSys = mContext->RenderSys();
	auto mScneMng = mContext->SceneMng();

	mTimer->Update();
	mInput->Frame();
	mContext->Update();

	//rotate camera
	auto camera0 = mScneMng->GetDefCamera();
	if (camera0/* && camera0->GetType() == kCameraPerspective*/)
	{
		auto camTranform = camera0->GetTransform();
		if (!mCameraInitInvLengthForward.any()) {
			mCameraInitInvLengthForward = -camera0->GetForward() * camera0->GetForwardLength();
			mCameraInitLookAt = camera0->GetLookAt();
		}
		
		float forwardLength = camera0->GetForwardLength();
		if (mInput->GetMouseWheelChange() != 0)
			forwardLength -= mInput->GetMouseWheelChange() * 0.8 * forwardLength;

		Eigen::Vector3f curInvForward = -camera0->GetForward();
		{
			Eigen::Vector2f m = mInput->GetMouseRightLocation();
			float mx = -3.14 * m.x();
			float my = -3.14 * 0.5 * m.y();

			Eigen::Quaternionf quat = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ())
				* Eigen::AngleAxisf(my, Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(mx, Eigen::Vector3f::UnitY());
			curInvForward = Transform3fAffine(quat) * curInvForward;
		}

		camera0->SetLookAt(mCameraInitLookAt + curInvForward * forwardLength, mCameraInitLookAt);
	}

	//rotate target
	{
		Eigen::Vector2f m = mInput->GetMouseLeftLocation();
		float mx = 3.14 * -m.x();
		float my = 3.14 * -m.y();
		auto quat = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ())
			* Eigen::AngleAxisf(my, Eigen::Vector3f::UnitX())
			* Eigen::AngleAxisf(mx, Eigen::Vector3f::UnitY()) 
			* mTransform->GetRotation();
		mTransform->SetRotation(quat);
	}

	renderSys->ClearFrameBuffer(nullptr, mBackgndColor, 1.0f, 0);
	OnRender();
}

std::string App::GetName()
{
	return mName;
}

void App::SetCaseIndex(int caseIndex)
{
	mCaseIndex = caseIndex;
}

Eigen::Matrix4f App::GetWorldTransform()
{
	return mTransform->GetSRT();
}

std::string& GetCurrentAppName() {
	static std::string gCurrentAppName;
	return gCurrentAppName;
}

typedef std::map<std::string, std::function<IApp*()>> AppCreatorByName;
AppCreatorByName& RegAppClasses() {
	static AppCreatorByName gRegAppClasses;
	return gRegAppClasses;
}
void RegisterApp(const char* name, std::function<IApp*()> appCls) {
	GetCurrentAppName() = name;
	RegAppClasses()[name] = appCls;
}

IApp* CreateApp(std::string name)
{
	auto entry = RegAppClasses()[name];
	assert(entry);
	IApp* gApp = entry();
	gApp->Create();
	return gApp;
}

void MirManager::SetMir(mir::Mir* ctx)
{
	mScneMng = ctx->SceneMng();
	mRendFac = ctx->RenderableFac();
	mResMng = ctx->ResourceMng();
	auto size = ctx->ResourceMng()->WinSize();
	mHalfSize = Eigen::Vector3f(size.x() / 2, size.y() / 2, 0);
	mWinCenter = Eigen::Vector3f(0, 0, 0);
}