#include <boost/algorithm/clamp.hpp>
#include <boost/filesystem.hpp>
#include "core/scene/transform.h"
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
	mTransform = mir::CreateInstance<mir::Transform>();
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
	mContext->ExecuteTaskSync(OnPostInitDevice());

	mInput = new mir::input::D3DInput(hInstance, hWnd, mContext->RenderSys()->WinSize().x(), mContext->RenderSys()->WinSize().y());
	mTimer = new mir::debug::Timer;
	return true;
}
void App::OnInitCamera()
{
	mContext->SceneMng()->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
}
void App::OnInitLight()
{
	mContext->SceneMng()->CreateAddLightNode<PointLight>();
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

	//rotate camera
	auto camera0 = mScneMng->GetDefCamera();
	if (camera0 && mControlCamera)
	{
		TransformPtr camera0Tranform = camera0->GetTransform();
		if (!mCameraInitInvLengthForward.any()) {
			mCameraInitInvLengthForward = -camera0Tranform->GetForward() * camera0Tranform->GetForwardLength();
			mCameraInitLookAt = camera0Tranform->GetLookAt();
		}
		
		float forwardLength = camera0Tranform->GetForwardLength();
		if (mInput->GetMouseWheelChange() != 0)
			forwardLength -= mInput->GetMouseWheelChange() * 0.8 * forwardLength;

		Eigen::Vector3f curInvForward = -camera0Tranform->GetForward();
		{
			Eigen::Vector2f m = mInput->GetMouseRightLocation();
			float mx = -3.14 * m.x();
			float my = -3.14 * 0.5 * m.y();
			(fabs(mx) > fabs(my)) ? my = 0 : mx = 0;

			Eigen::Quaternionf quat = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ())
				* Eigen::AngleAxisf(my, Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(mx, Eigen::Vector3f::UnitY());
			curInvForward = Transform3fAffine(quat) * curInvForward;
		}

		camera0->SetLookAt(mCameraInitLookAt + curInvForward * forwardLength, mCameraInitLookAt);
	}

	//rotate target
	if (mTransform)
	{
		Eigen::Vector2f m = mInput->GetMouseLeftLocation();
		float mx = 3.14 * -m.x();
		float my = 3.14 * -m.y();
		(fabs(mx) > fabs(my)) ? my = 0 : mx = 0;

		auto quat = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ())
			* Eigen::AngleAxisf(my, Eigen::Vector3f::UnitX())
			* Eigen::AngleAxisf(mx, Eigen::Vector3f::UnitY()) 
			* mTransform->GetRotation();
		mTransform->SetRotation(quat);

		if (mInput->IsKeyPressed(DIK_UPARROW)) {
			mTransform->SetPosition(mTransform->GetPosition() + Eigen::Vector3f(0, 0, 5));
		}
		else if (mInput->IsKeyPressed(DIK_DOWNARROW)) {
			mTransform->SetPosition(mTransform->GetPosition() - Eigen::Vector3f(0, 0, 5));
		}
	}
	mContext->Update(mTimer->mDeltaTime);

	OnRender();
}
void App::OnRender()
{
	mContext->Render();
}

std::string App::GetName()
{
	return mName;
}

void App::SetCaseIndex(int caseIndex)
{
	mCaseIndex = caseIndex;
}
void App::SetCaseSecondIndex(int secondIndex)
{
	mCaseSecondIndex = secondIndex;
}

Eigen::Matrix4f App::GetWorldTransform()
{
	return mTransform->GetWorldMatrix();
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

	float aspect = 1.0 * size.x() / size.y();
	mCamWinHSize = Eigen::Vector3f(aspect * 5, 5, 0);
}

void MirManager::SetPPU(float ppu)
{
	mScneMng->SetPixelPerUnit(ppu);

	auto size = mScneMng->GetDefCamera()->GetOthoWinSize();
	mCamWinHSize = Eigen::Vector3f(size.x() / 2, size.y() / 2, 0);
}
