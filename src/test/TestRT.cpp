#include "TestCase.h"
#if defined TEST_RT && TEST_CASE == TEST_RT
#include "TApp.h"
#include "ISceneManager.h"
#include "rendersys/TInterfaceType.h"
#include "TAssimpModel.h"
#include "TSprite.h"
#include "TTransform.h"
#include "Utility.h"

class TestRT : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	TAssimpModel* mModel = nullptr;
	IRenderTexturePtr mRendTexture = nullptr;
	TSpritePtr mSprite, mLayerColor;
};

/********** TestRT **********/
void TestRT::OnInitLight()
{
	auto light1 = mContext->GetSceneMng()->AddPointLight();
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.01, 0);
	light1->SetSpecularPower(60);
}

void TestRT::OnPostInitDevice()
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> layouts =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mModel = new TAssimpModel(mRenderSys, mMove, MAKE_MAT_NAME("Lesson3.3"), layouts);
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01); mMove->SetPosition(0, 0, 0);
	
	mRendTexture = mRenderSys->CreateRenderTexture(mRenderSys->GetWinSize().x, mRenderSys->GetWinSize().y);

	mSprite = std::make_shared<TSprite>(mRenderSys, E_MAT_SPRITE);
	mSprite->SetTexture(mRendTexture->GetColorTexture());
	mSprite->SetPosition(0, 0, 0);
	mSprite->SetSize(5,5);

	/*mLayerColor = std::make_shared<TSprite>(mRenderSys, E_MAT_LAYERCOLOR);
	mLayerColor->SetPosition(-5, -5, 0);
	mLayerColor->SetSize(5, 5);*/
}

//#define USE_RENDER_TARGET
void TestRT::OnRender()
{
	if (mContext->GetRenderSys()->BeginScene()) {
#ifdef USE_RENDER_TARGET
		mRenderSys->SetRenderTarget(mRendTexture);
		mRenderSys->ClearColorDepthStencil(XMFLOAT4(0, 0, 0, 0));
#endif
		mModel->Update(mTimer->mDeltaTime);
		mModel->Draw();
#ifdef USE_RENDER_TARGET
		mRenderSys->SetRenderTarget(nullptr);
		if (mSprite) mSprite->Draw();
		if (mLayerColor) mLayerColor->Draw();
#endif
		mContext->GetRenderSys()->EndScene();
	}
}

auto reg = AppRegister<TestRT>("TestRT: RenderTarget");
#endif