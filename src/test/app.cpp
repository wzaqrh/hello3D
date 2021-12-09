#include <boost/algorithm/clamp.hpp>
#include <boost/filesystem.hpp>
#include "core/base/transform.h"
#include "core/base/input.h"
#include "core/rendersys/render_system.h"
#include "core/scene/scene_manager.h"
#include "test/app.h"

using namespace mir;

//#define AppLaunchMode __LaunchSync__
#define AppLaunchMode __LaunchAsync__

App::App()
{
	mContext = nullptr;
	mInput = nullptr;
	mTimer = nullptr;
	mMoveDefScale = 1.0f;

	mTransform = std::make_shared<mir::Transform>();
	//mBackgndColor = Eigen::Vector4f(0.5f, 0.5f, 0.5f, 1.0f);
	mBackgndColor = Eigen::Vector4f(0.3f, 0.3f, 0.3f, 0.0f);
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
	OnInitCamera();
	OnInitLight();
	OnPostInitDevice();

	mInput = new mir::input::D3DInput(hInstance, hWnd, mContext->RenderSys()->WinSize().x(), mContext->RenderSys()->WinSize().y());
	mTimer = new mir::debug::Timer;
	mOriginCameraDistance = 0;
	return true;
}
void App::OnInitCamera()
{
	mContext->SceneMng()->AddPerspectiveCamera();
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
	auto sceneMng = mContext->SceneMng();

	mTimer->Update();
	mInput->Frame();
	mContext->Update();

	auto camera0 = sceneMng->GetDefCamera();
	//rotate camera
	if (camera0 && camera0->GetType() == kCameraPerspective)
	{
		if (mOriginCameraDistance == 0)
			mOriginCameraDistance = camera0->GetForwardLength();

		float eyeDistance = camera0->GetForwardLength();
		if (mInput->GetMouseWheel() != 0) {
			float wheel = boost::algorithm::clamp(mInput->GetMouseWheel() / 1000.0, -1, 1);
			eyeDistance -= wheel * 0.8 * eyeDistance;
		}

	#if 1
		Eigen::Vector2i m = mInput->GetMouseRightLocation();
		float eulerX = 3.14 * m.x() / renderSys->WinSize().x();
		float eulerY = 3.14 * m.y() / renderSys->WinSize().y();
		camera0->GetTransform()->SetEuler(Eigen::Vector3f(0, eulerX, eulerY));
	#else
		Eigen::Vector3f tpos = Eigen::Vector3f(0, 0, -eyeDistance) + camera0->GetLookAt();

		Eigen::Vector3f cpos;
		{
			Eigen::Vector2i m = mInput->GetMouseRightLocation();
			float eulerY = 3.14 * m.x() / renderSys->WinSize().x();
			float eulerX = 3.14 * m.y() / renderSys->WinSize().y();

			Eigen::Quaternion<float> euler = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ())
				* Eigen::AngleAxisf(eulerX, Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(eulerY, Eigen::Vector3f::UnitY());
			cpos = Transform3fAffine(euler) * tpos;
		}

		camera0->SetLookAt(cpos, camera0->GetLookAt());
	#endif
	}

	//rotate target
	{
		Eigen::Vector2i m = mInput->GetMouseLeftLocation();
		float angy = 3.14 * -m.x() / renderSys->WinSize().x(), angx = 3.14 * -m.y() / renderSys->WinSize().y();
		mTransform->SetEuler(Eigen::Vector3f(angx, angy, 0));

		//float scalez = boost::algorithm::clamp(mMoveDefScale * (1000 + m.z()) / 1000.0f, 0.00001f, 10.0f);
		//mTransform->SetScale(Eigen::Vector3f(scalez, scalez, scalez));
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
