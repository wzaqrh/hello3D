#include "Lesson7.h"

void Lesson7::OnInitLight()
{
	auto light = mRenderSys->AddDirectLight();
	light->SetDirection(1, -1, 1);
}

#define USE_RENDER_TEXTURE
void Lesson7::OnPostInitDevice()
{
	mRenderSys->SetCamera(45, 150, 1000);

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson3.3.fx", "shader\\Lesson3.3.fx");
	//mModel = new AssimpModel(mRenderSys, "shader\\ShadowMap.fx", "shader\\ShadowMap.fx");
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mScale = 0.1; mPosition = XMFLOAT3(0, 0, 0);

	mRendTexture = mRenderSys->CreateRenderTexture(mRenderSys->mScreenWidth, mRenderSys->mScreenHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);

	mSprite = std::make_shared<TSprite>(mRenderSys, "shader\\Sprite.fx", "shader\\Sprite.fx");
	mSprite->SetTexture(mRendTexture->mRenderTargetSRV);
	int w = 50;
	int h = 50;
	mSprite->SetPosition(-w, -h, 0);
	mSprite->SetSize(w*2, h*2);
}

void Lesson7::OnRender()
{
#ifdef USE_RENDER_TEXTURE
	mRenderSys->ClearRenderTexture(mRendTexture, XMFLOAT4(0.125f, 0.125f, 0.3f, 1.0f));
	mRenderSys->SetRenderTarget(mRendTexture);
#endif
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
#ifdef USE_RENDER_TEXTURE
	mRenderSys->SetRenderTarget(nullptr);
	mSprite->Draw();
#endif
}

auto reg = AppRegister<Lesson7>("Lesson7: ShadowMap");