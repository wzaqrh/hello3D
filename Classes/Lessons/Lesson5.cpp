#include "Lesson5.h"
#include "TInterfaceType.h"

/********** Lesson5 **********/
void Lesson5::OnInitLight()
{
	auto light1 = mRenderSys->AddPointLight();
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.01, 0);
	light1->SetSpecularPower(60);
}

void Lesson5::OnPostInitDevice()
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
void Lesson5::OnRender()
{
	if (mRenderSys->BeginScene()) {
#ifdef USE_RENDER_TARGET
		mRenderSys->SetRenderTarget(mRendTexture);
		mRenderSys->ClearColorDepthStencil(XMFLOAT4(0, 0, 0, 0));
#endif
		mModel->Update(mTimer.mDeltaTime);
		mModel->Draw();
#ifdef USE_RENDER_TARGET
		mRenderSys->SetRenderTarget(nullptr);
		if (mSprite) mSprite->Draw();
		if (mLayerColor) mLayerColor->Draw();
#endif
		mRenderSys->EndScene();
	}
}

//auto reg = AppRegister<Lesson5>("TAppLesson5: RenderTarget");