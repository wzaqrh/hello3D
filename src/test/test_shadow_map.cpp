#include "test/test_case.h"
#if defined TEST_SHADOW_MAP && TEST_CASE == TEST_SHADOW_MAP
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

struct cbShadowMap
{
	XMMATRIX LightView;
	XMMATRIX LightProjection;
};

class TestShadowMap : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModel *mModel1, *mModel2 = nullptr;
	TPointLightPtr mLight;
};

void TestShadowMap::OnInitLight()
{
	mLight = mContext->GetSceneMng()->AddPointLight();//1, -1, 1
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
	mContext->GetSceneMng()->SetPerspectiveCamera(45, 30, 300);
	mContext->GetSceneMng()->SetSkyBox("images\\uffizi_cross.dds");

	float dd = 3;
	mMove->SetDefScale(SCALE_BASE * 0.2); mMove->SetPosition(0, 0, -dd);
#if 0
	auto LightCam = mLight->GetLightCamera(*mContext->GetSceneMng()->GetDefCamera());
	//auto pCam = &LightCam;
	auto pCam = mContext->GetSceneMng()->GetDefCamera();
	XMFLOAT4 p0 = pCam->CalNDC(XMFLOAT4(30, 30, 0, 1));
	//XMFLOAT3 p1 = pCam->CalNDC(mPosition);
#endif
#else
	mScale = 0.01;
#endif
	
	//std::string matName = E_MAT_MODEL_PBR;
	std::string matName = E_MAT_MODEL;

	auto move1 = std::make_shared<Movable>();
	move1->SetScale(SCALE_BASE);
	mModel1 = new AssimpModel(mContext->GetRenderSys(), move1, matName);
	gModelPath = "Spaceship\\"; mModel1->LoadModel(MakeModelPath("Spaceship.fbx"));

	mModel2 = new AssimpModel(mContext->GetRenderSys(), mMove, matName);
	gModelPath = "Spaceship\\"; mModel2->LoadModel(MakeModelPath("Spaceship.fbx"));
}

void TestShadowMap::OnRender()
{
#if 1
	mModel1->Update(mTimer->mDeltaTime);
	mModel2->Update(mTimer->mDeltaTime);

	RenderOperationQueue opQueue;
	mModel1->GenRenderOperation(opQueue);
	mModel2->GenRenderOperation(opQueue);

	if (mContext->GetRenderSys()->BeginScene()) {
		mContext->GetRenderSys()->RenderQueue(opQueue, E_PASS_SHADOWCASTER);
		mContext->GetRenderSys()->RenderQueue(opQueue, E_PASS_FORWARDBASE);
		mContext->GetRenderSys()->EndScene();
	}
#else
	//pass1
	mRenderSys->SetRenderTarget(mPass1RT);
	mRenderSys->ClearColorDepthStencil(mPass1RT, XMFLOAT4(1, 1, 1, 1.0f));
	auto LightCam = mLight->GetLightCamera(*mContext->GetSceneMng()->GetDefCamera());
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

auto reg = AppRegister<TestShadowMap>("Lesson7: ShadowMap");
#endif