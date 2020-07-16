#include "TestCase.h"
#if TEST_CASE == TEST_SHADOW_MAP
#include "TApp.h"
#include "ISceneManager.h"
#include "TAssimpModel.h"
#include "TSprite.h"
#include "TTransform.h"
#include "Utility.h"

struct cbShadowMap
{
	XMMATRIX LightView;
	XMMATRIX LightProjection;
};

class TestShadowMap : public TApp
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	TAssimpModel *mModel1, *mModel2 = nullptr;
	TPointLightPtr mLight;
};

void TestShadowMap::OnInitLight()
{
	mLight = mRenderSys->GetSceneManager()->AddPointLight();//1, -1, 1
	float ddd = 10;
	mLight->SetPosition(ddd, ddd, -ddd);
	mLight->SetAttenuation(1, 0.001, 0);
	//mLight->SetPosition(0, 0, -150);
}

#define USE_RENDER_TEXTURE
#define SCALE_BASE 0.01
void TestShadowMap::OnPostInitDevice()
{
#if 1
	mRenderSys->GetSceneManager()->SetPerspectiveCamera(45, 30, 300);
	mRenderSys->GetSceneManager()->SetSkyBox("images\\uffizi_cross.dds");

	float dd = 9;
	mMove->SetDefScale(SCALE_BASE * 0.02); mMove->SetPosition(dd, dd, -dd);
#if 0
	auto LightCam = mLight->GetLightCamera(*mRenderSys->GetSceneManager()->GetDefCamera());
	//auto pCam = &LightCam;
	auto pCam = mRenderSys->GetSceneManager()->GetDefCamera();
	XMFLOAT4 p0 = pCam->CalNDC(XMFLOAT4(30, 30, 0, 1));
	//XMFLOAT3 p1 = pCam->CalNDC(mPosition);
#endif
#else
	mScale = 0.01;
#endif
	
	//std::string matName = E_MAT_MODEL_PBR;
	std::string matName = E_MAT_MODEL;

	auto move1 = std::make_shared<TMovable>();
	move1->SetScale(SCALE_BASE);
	mModel1 = new TAssimpModel(mRenderSys, move1, matName);
	gModelPath = "Spaceship\\"; mModel1->LoadModel(MakeModelPath("Spaceship.fbx"));

	mModel2 = new TAssimpModel(mRenderSys, mMove, matName);
	gModelPath = "Spaceship\\"; mModel2->LoadModel(MakeModelPath("Spaceship.fbx"));
}

void TestShadowMap::OnRender()
{
#if 1
	mModel1->Update(mTimer->mDeltaTime);
	mModel2->Update(mTimer->mDeltaTime);

	TRenderOperationQueue opQueue;
	mModel1->GenRenderOperation(opQueue);
	mModel2->GenRenderOperation(opQueue);

	if (mRenderSys->BeginScene()) {
		mRenderSys->RenderQueue(opQueue, E_PASS_SHADOWCASTER);
		mRenderSys->RenderQueue(opQueue, E_PASS_FORWARDBASE);
		mRenderSys->EndScene();
	}
#else
	//pass1
	mRenderSys->SetRenderTarget(mPass1RT);
	mRenderSys->ClearColorDepthStencil(mPass1RT, XMFLOAT4(1, 1, 1, 1.0f));
	auto LightCam = mLight->GetLightCamera(*mRenderSys->GetSceneManager()->GetDefCamera());
	{
		mModel1->Update(mTimer.mDeltaTime);
		float s = SCALE_BASE;
		mRenderSys->ApplyMaterial(mModel1->mMaterial, XMMatrixScaling(s, s, s), &LightCam);
		mModel1->Draw();

		mModel2->Update(mTimer.mDeltaTime);
		mRenderSys->ApplyMaterial(mModel2->mMaterial, GetWorldTransform(), &LightCam);
		mModel2->Draw();
	}
	mRenderSys->SetRenderTarget(nullptr);

	mRenderSys->UpdateConstBuffer(mModel1->mMaterial->CurTech()->mPasses[0]->mConstantBuffers[2], &cb);
	mRenderSys->ApplyMaterial(mModel1->mMaterial, XMMatrixScaling(s, s, s), nullptr, mProgShadowMap);
	mModel1->DrawShadow(mPass1RT->mRenderTargetSRV);

	mRenderSys->UpdateConstBuffer(mModel2->mMaterial->CurTech()->mPasses[0]->mConstantBuffers[2], &cb);
	mRenderSys->ApplyMaterial(mModel2->mMaterial, GetWorldTransform(), nullptr, mProgShadowMap);
	mModel2->DrawShadow(mPass1RT->mRenderTargetSRV);
#endif
}

auto reg = AppRegister<Lesson7>("Lesson7: ShadowMap");
#endif