#include "TApp.h"
#include "Utility.h"
#include "TRenderSystem.h"

TApp::TApp()
{
	mRenderSys = new TRenderSystem;
	mMove = std::make_shared<TMovable>();
	mBackgndColor = XMFLOAT4(0.0f, 0.125f, 0.3f, 1.0f);
}

TApp::~TApp()
{
	delete mRenderSys;
}

void TApp::Attach(HINSTANCE hInstance, HWND hWnd)
{
	mRenderSys->mHInst = hInstance;
	mRenderSys->mHWnd = hWnd;
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
	float ClearColor[4] = { mBackgndColor.x, mBackgndColor.y, mBackgndColor.z, mBackgndColor.w }; // red, green, blue, alpha
	mRenderSys->mDeviceContext->ClearRenderTargetView(mRenderSys->mBackRenderTargetView, ClearColor);
	mRenderSys->mDeviceContext->ClearDepthStencilView(mRenderSys->mBackDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

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

	mRenderSys->mSwapChain->Present(0, 0);
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
	return gApp;
}
