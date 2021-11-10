#include "test/app.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"
#include "core/base/transform.h"
#include "core/rendersys/scene_manager.h"
#include "core/base/utility.h"

using namespace mir;

App::App()
{
	mMove = MakePtr<mir::Movable>();
	mBackgndColor = XMFLOAT4(0.0f, 0.125f, 0.3f, 1.0f);
	mContext = new mir::Mir;
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
	OnInitLight();
	OnPostInitDevice();

	mInput = new mir::D3DInput(hInstance, hWnd, mContext->RenderSys()->GetWinSize().x, mContext->RenderSys()->GetWinSize().y);
	mTimer = new mir::Timer;
	return true;
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

	renderSys->ClearColorDepthStencil(mBackgndColor, 1.0f, 0);

	mTimer->Update();
	mInput->Frame();
	renderSys->Update(0);
	//rotate camera
	if (sceneMng->GetDefCamera()->mIsPespective)
	{
		mir::Int4 m = mInput->GetMouseLocation(false);
		float angy = 3.14 * m.x / renderSys->GetWinSize().x, angx = 3.14 * m.y / renderSys->GetWinSize().y;
		XMMATRIX euler = XMMatrixRotationZ(0) * XMMatrixRotationX(angx) * XMMatrixRotationY(angy);

		auto eye = sceneMng->GetDefCamera()->mEyePos; 
		float eyeDistance = -sqrt(eye.x*eye.x + eye.y*eye.y + eye.z*eye.z);
		XMVECTOR vec = XMVector3Transform(XMVectorSet(0, 0, eyeDistance, 1), euler);
		eye = XMFLOAT3(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec));
		sceneMng->GetDefCamera()->SetLookAt(eye, sceneMng->GetDefCamera()->mLookAtPos);
	}

	{
		mir::Int4 m = mInput->GetMouseLocation(true);
		float scalez = mir::clamp(0.00001f, 10.0f, mMove->mDefScale * (1000 + m.z) / 1000.0f);
		float angy = 3.14 * -m.x / renderSys->GetWinSize().x, angx = 3.14 * -m.y / renderSys->GetWinSize().y;
		
		mMove->SetScale(scalez);
		mMove->SetEulerX(angx);
		mMove->SetEulerY(angy);
	}

	OnRender();
}

std::string App::GetName()
{
	return mName;
}
XMMATRIX App::GetWorldTransform()
{
	return mMove->GetWorldTransform();
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
