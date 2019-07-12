#include "TApp.h"
#include "Utility.h"
#include "TRenderSystem.h"

TApp::TApp()
{
	mRenderSys = new TRenderSystem;
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
	OnPostInitDevice();
	return true;
}

void TApp::CleanUp()
{
	mRenderSys->CleanUp();
}

void TApp::Render()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	mRenderSys->mDeviceContext->ClearRenderTargetView(mRenderSys->mRenderTargetView, ClearColor);
	mRenderSys->mDeviceContext->ClearDepthStencilView(mRenderSys->mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	mTimer.Update();
	InputFrame();
	OnRender();

	mRenderSys->mSwapChain->Present(0, 0);
}

std::string TApp::GetName()
{
	return mName;
}

XMMATRIX TApp::GetWorldTransform()
{
	float rY = (90) / 180.0 * 3.14;
	XMFLOAT3 t = XMFLOAT3(0, 0, 0);
	float scale = 1.0;

	int mx, my;
	InputGetMouseLocation(&mx, &my);
	float angy = 3.14 * -mx / mRenderSys->mScreenWidth, angx = 3.14 * -my / mRenderSys->mScreenHeight;

	XMMATRIX euler = XMMatrixRotationZ(0) * XMMatrixRotationX(angx) * XMMatrixRotationY(angy);
	return euler
		* XMMatrixTranslation(t.x, t.y, t.z)
		* XMMatrixScaling(scale, scale, scale);
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
	gApp = entry();
	return gApp;
}
