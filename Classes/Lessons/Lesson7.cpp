#include "Lesson7.h"

void Lesson7::OnInitLight()
{
	mLight = mRenderSys->AddPointLight();//1, -1, 1
	//mLight->SetPosition(150,150,-150);
	mLight->SetPosition(0, 0, -150);
}

//#define USE_RENDER_TEXTURE
void Lesson7::OnPostInitDevice()
{
	mRenderSys->SetCamera(45, 150, 1000);

	{
		gModelPath = "Spaceship\\"; mScale = 0.002; mPosition = XMFLOAT3(0, 0, -145);

		mModel1 = new AssimpModel(mRenderSys, "shader\\ShadowDepth.fx", "shader\\ShadowDepth.fx");
		mModel1->LoadModel(MakeModelPath("Spaceship.fbx"));

		mModel2 = new AssimpModel(mRenderSys, "shader\\ShadowDepth.fx", "shader\\ShadowDepth.fx");
		mModel2->LoadModel(MakeModelPath("Spaceship.fbx"));

		mPass1RT = mRenderSys->CreateRenderTexture(mRenderSys->mScreenWidth, mRenderSys->mScreenHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
	}

	mProgShadowMap = mRenderSys->CreateProgram("shader\\ShadowMap.fx", "shader\\ShadowMap.fx");

	//mSecondPass = std::make_shared<TSprite>(mRenderSys, "shader\\ShadowMap.fx", "shader\\ShadowMap.fx");
	mSecondPass = std::make_shared<TSprite>(mRenderSys, "shader\\Sprite.fx", "shader\\Sprite.fx");
	mSecondPass->SetTexture(mPass1RT->mRenderTargetSRV);
	int w = 50;
	int h = 50;
	mSecondPass->SetPosition(-w, -h, 0);
	mSecondPass->SetSize(w*2, h*2);
	//mSecondPass->SetFlipY(true);

	auto Pass2Mat = mSecondPass->mMaterial;
	Pass2Mat->AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(cbShadowMap)));
}

void Lesson7::OnRender()
{
#ifdef USE_RENDER_TEXTURE
	mRenderSys->ClearRenderTexture(mPass1RT, XMFLOAT4(0.125f, 0.125f, 0.3f, 1.0f));
	mRenderSys->SetRenderTarget(mPass1RT);
#endif
	auto LightCam = mLight->GetLightCamera(*mRenderSys->mDefCamera);

	mModel1->Update(mTimer.mDeltaTime);	
	mRenderSys->ApplyMaterial(mModel1->mMaterial, XMMatrixScaling(0.1, 0.1, 0.1), &LightCam);
	mModel1->Draw();

	mModel2->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel2->mMaterial, GetWorldTransform(), &LightCam);
	mModel2->Draw();
#ifdef USE_RENDER_TEXTURE
	mRenderSys->SetRenderTarget(nullptr);

	cbShadowMap cb;
	cb.LightProjection = LightCam.mProjection;
	cb.LightView = LightCam.mView;
	mRenderSys->UpdateConstBuffer(mSecondPass->mMaterial->mConstantBuffers[2], &cb);
	
	mSecondPass->Draw();
#endif
}

auto reg = AppRegister<Lesson7>("Lesson7: ShadowMap");