#include "Lesson7.h"

void Lesson7::OnInitLight()
{
	mLight = mRenderSys->AddPointLight();//1, -1, 1
	float ddd = 10;
	mLight->SetPosition(ddd, ddd, -ddd);
	mLight->SetAttenuation(1, 0.001, 0);
	//mLight->SetPosition(0, 0, -150);
}

#define USE_RENDER_TEXTURE
#define SCALE_BASE 0.01
void Lesson7::OnPostInitDevice()
{
	mRenderSys->SetCamera(45, 30, 300);

	{
		float dd = 9;
		gModelPath = "Spaceship\\"; mScale = SCALE_BASE * 0.02; mPosition = XMFLOAT3(dd, dd, -dd);

#if 1
		auto LightCam = mLight->GetLightCamera(*mRenderSys->mDefCamera);
		auto pCam = &LightCam;
		//auto pCam = mRenderSys->mDefCamera;
		XMFLOAT3 p0 = pCam->CalNDC(XMFLOAT3(0, 0, 0));
		XMFLOAT3 p1 = pCam->CalNDC(mPosition);
#endif

		mModel1 = new AssimpModel(mRenderSys, "shader\\ShadowDepth.fx", "shader\\ShadowDepth.fx", [&](TMaterialPtr mat) {
			mat->AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(cbShadowMap)));
		});
		mModel1->LoadModel(MakeModelPath("Spaceship.fbx"));

		mModel2 = new AssimpModel(mRenderSys, "shader\\ShadowDepth.fx", "shader\\ShadowDepth.fx", [&](TMaterialPtr mat) {
			mat->AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(cbShadowMap)));
		});
		mModel2->LoadModel(MakeModelPath("Spaceship.fbx"));

		mPass1RT = mRenderSys->CreateRenderTexture(mRenderSys->mScreenWidth, mRenderSys->mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	}

	mProgShadowMap = mRenderSys->CreateProgram("shader\\ShadowMap.fx", "shader\\ShadowMap.fx");
#if 0
	mSecondPass = std::make_shared<TSprite>(mRenderSys, "shader\\Sprite.fx", "shader\\Sprite.fx");
	mSecondPass->SetTexture(mPass1RT->mRenderTargetSRV);
	int w = 50;
	int h = 50;
	mSecondPass->SetPosition(-w, -h, 0);
	mSecondPass->SetSize(w*2, h*2);
#endif
}

void Lesson7::OnRender()
{
#ifdef USE_RENDER_TEXTURE
	mRenderSys->ClearRenderTexture(mPass1RT, XMFLOAT4(1, 1, 1, 1.0f));
	mRenderSys->SetRenderTarget(mPass1RT);
#endif
	auto LightCam = mLight->GetLightCamera(*mRenderSys->mDefCamera);

	mModel1->Update(mTimer.mDeltaTime);
	float s = SCALE_BASE;
#ifndef USE_RENDER_OP
	mRenderSys->ApplyMaterial(mModel1->mMaterial, XMMatrixScaling(s, s, s), &LightCam);
#endif
	mModel1->Draw();

	mModel2->Update(mTimer.mDeltaTime);
#ifndef USE_RENDER_OP
	mRenderSys->ApplyMaterial(mModel2->mMaterial, GetWorldTransform(), &LightCam);
#endif
	mModel2->Draw();
#ifdef USE_RENDER_TEXTURE
	mRenderSys->SetRenderTarget(nullptr);

	//pass2 //need refactor
	/*cbShadowMap cb = { LightCam.mView, LightCam.mProjection };
	mRenderSys->UpdateConstBuffer(mModel1->mMaterial->CurTech()->mPasses[0]->mConstantBuffers[2], &cb);
	mRenderSys->ApplyMaterial(mModel1->mMaterial, XMMatrixScaling(s, s, s), nullptr, mProgShadowMap);
	mModel1->DrawShadow(mPass1RT->mRenderTargetSRV);

	mRenderSys->UpdateConstBuffer(mModel2->mMaterial->CurTech()->mPasses[0]->mConstantBuffers[2], &cb);
	mRenderSys->ApplyMaterial(mModel2->mMaterial, GetWorldTransform(), nullptr, mProgShadowMap);
	mModel2->DrawShadow(mPass1RT->mRenderTargetSRV);*/
#endif
}

//auto reg = AppRegister<Lesson7>("Lesson7: ShadowMap");