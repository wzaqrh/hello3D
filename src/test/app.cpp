#include "test/app.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"
#include "core/base/transform.h"
#include "core/rendersys/scene_manager.h"
#include "core/base/utility.h"

using namespace mir;

TApp::TApp()
{
	mMove = mir::MakePtr<mir::TMovable>();
	mBackgndColor = XMFLOAT4(0.0f, 0.125f, 0.3f, 1.0f);
	mContext = new mir::TContext;
}
void TApp::Create()
{}
bool TApp::Initialize(HINSTANCE hInstance, HWND hWnd)
{
	mHnd = hWnd;

	OnPreInitDevice();
	if (!mContext->Initialize(mHnd)) {
		mContext->Dispose();
		return false;
	}
	OnInitLight();
	OnPostInitDevice();

	mInput = new mir::TD3DInput(hInstance, hWnd, mContext->GetRenderSys()->GetWinSize().x, mContext->GetRenderSys()->GetWinSize().y);
	mTimer = new mir::SDTimer;
	return true;
}

TApp::~TApp()
{
	delete mContext;
}
void TApp::CleanUp()
{
	mContext->Dispose();
}

void TApp::Render()
{
	auto renderSys = mContext->GetRenderSys();
	auto sceneMng = mContext->GetSceneMng();

	renderSys->ClearColorDepthStencil(mBackgndColor, 1.0f, 0);

	mTimer->Update();
	mInput->Frame();
	renderSys->Update(0);
	//rotate camera
	if (sceneMng->GetDefCamera()->mIsPespective)
	{
		mir::TINT4 m = mInput->GetMouseLocation(false);
		float angy = 3.14 * m.x / renderSys->GetWinSize().x, angx = 3.14 * m.y / renderSys->GetWinSize().y;
		XMMATRIX euler = XMMatrixRotationZ(0) * XMMatrixRotationX(angx) * XMMatrixRotationY(angy);

		auto eye = sceneMng->GetDefCamera()->mEye;
		XMVECTOR vec = XMVector3Transform(XMVectorSet(0, 0, -sceneMng->GetDefCamera()->mEyeDistance, 1), euler);
		eye = XMFLOAT3(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec));
		sceneMng->GetDefCamera()->SetLookAt(eye, sceneMng->GetDefCamera()->mAt);
	}

	{
		mir::TINT4 m = mInput->GetMouseLocation(true);
		float scalez = mir::clamp(0.00001f, 10.0f, mMove->mDefScale * (1000 + m.z) / 1000.0f);
		float angy = 3.14 * -m.x / renderSys->GetWinSize().x, angx = 3.14 * -m.y / renderSys->GetWinSize().y;
		
		mMove->SetScale(scalez);
		mMove->SetEulerX(angx);
		mMove->SetEulerY(angy);
	}

	OnRender();
}

std::string TApp::GetName()
{
	return mName;
}

void TApp::OnInitLight()
{
	mContext->GetSceneMng()->AddPointLight();
}

XMMATRIX TApp::GetWorldTransform()
{
	return mMove->GetWorldTransform();
}


std::map<std::string, std::function<IApp*()>> gRegAppClasses;
void RegisterApp(std::string name, std::function<IApp*()> appCls) {
	gRegAppClasses[name] = appCls;
}

std::string gCurrentAppName;
IApp* CreateApp(std::string name)
{
	auto entry = gRegAppClasses[name];
	assert(entry);
	IApp* gApp = entry();
	gApp->Create();
	return gApp;
}
