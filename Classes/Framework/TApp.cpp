#include "TApp.h"
#include "Utility.h"
#include "IRenderSystem.h"
#include "TRenderSystem11.h"

TApp::TApp()
{
	mMove = std::make_shared<TMovable>();
	mBackgndColor = XMFLOAT4(0.0f, 0.125f, 0.3f, 1.0f);
}

TApp::~TApp()
{
	delete mRenderSys;
}

void TApp::Create()
{
	mRenderSys = OnCreateRenderSys();
}

IRenderSystem* TApp::OnCreateRenderSys()
{
	return new TRenderSystem11;
}

void TApp::Attach(HINSTANCE hInstance, HWND hWnd)
{
	mRenderSys->SetHandle(hInstance, hWnd);
}

bool TApp::Initialize()
{
	OnPreInitDevice();
	if (FAILED(mRenderSys->Initialize())) {
		mRenderSys->CleanUp();
		return false;
	}

	OnInitLight();

	OnPostInitDevice();
	return true;
}

void TApp::CleanUp()
{
	mRenderSys->CleanUp();
}

void TApp::Render()
{
	mRenderSys->ClearColorDepthStencil(mBackgndColor, 1.0f, 0);

	mTimer.Update();
	mRenderSys->mInput->Frame();
	mRenderSys->Update(0);
	//rotate camera
	{
		TINT4 m = mRenderSys->mInput->GetMouseLocation(false);
		float angy = 3.14 * m.x / mRenderSys->mScreenWidth, angx = 3.14 * m.y / mRenderSys->mScreenHeight;
		XMMATRIX euler = XMMatrixRotationZ(0) * XMMatrixRotationX(angx) * XMMatrixRotationY(angy);

		auto eye = mRenderSys->mDefCamera->mEye;
		XMVECTOR vec = XMVector3Transform(XMVectorSet(0, 0, -mRenderSys->mDefCamera->mEyeDistance, 1), euler);
		mRenderSys->mDefCamera->SetLookAt(XMFLOAT3(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec)), mRenderSys->mDefCamera->mAt);
	}

	{
		TINT4 m = mRenderSys->mInput->GetMouseLocation(true);
		float scalez = clamp(0.00001f, 10.0f, mMove->mDefScale * (1000 + m.z) / 1000.0f);
		float angy = 3.14 * -m.x / mRenderSys->mScreenWidth, angx = 3.14 * -m.y / mRenderSys->mScreenHeight;
		
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
	mRenderSys->AddPointLight();
}

XMMATRIX TApp::GetWorldTransform()
{
	return mMove->GetWorldTransform();
}

std::map<std::string, std::function<TApp*()>> gRegAppClasses;
void RegisterApp(std::string name, std::function<TApp*()> appCls)
{
	gRegAppClasses[name] = appCls;
}

std::string gCurrentAppName;
TApp* gApp;
TApp* CreateApp(std::string name)
{
	auto entry = gRegAppClasses[name];
	assert(entry);
	gApp = entry();
	gApp->Create();
	return gApp;
}
