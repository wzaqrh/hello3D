#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/scene_manager.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/renderable.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"

using namespace mir;

class TestPBR : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModelPtr mModel = nullptr;
};

void TestPBR::OnInitLight()
{
#if 0
	{
		auto light = mContext->GetSceneMng()->AddPointLight();//1, -1, 1
		float ddd = 10;
		light->SetPosition(-ddd, ddd, -ddd);
		light->SetAttenuation(1, 0.001, 0);
	}
#endif

#if 0
	{
		auto light = mContext->GetSceneMng()->AddDirectLight();
		light->SetDirection(0, -1, 0);
		float i = 0.7;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
#endif

#if 1
	{
		auto light = mContext->SceneMng()->AddDirectLight();
		light->SetDirection(1, -1, 1);
		float i = 0.7;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
	{
		auto light = mContext->SceneMng()->AddDirectLight();
		light->SetDirection(-1, -1, 0);
		float i = 1.0;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
#endif

#if 0
	{
		auto light = mContext->GetSceneMng()->AddDirectLight();
		light->SetDirection(1, -1, 1);
		float i = 1.0;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
	{
		auto light = mContext->GetSceneMng()->AddDirectLight();
		light->SetDirection(-1, -1, -1);
		float i = 1.0;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
#endif
}

//#define PBR_DEBUG
/********** Lesson6 **********/
void TestPBR::OnPostInitDevice()
{
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddPerspectiveCamera(Eigen::Vector3f(0, 0, -30), 1000, 45);

	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("model/uffizi_cross.dds"));
	TIME_PROFILE(Lesson6_OnPostInitDevice);

	mModel = mContext->RenderableFac()->CreateAssimpModel(mTransform, E_MAT_MODEL_PBR);
	mModel->LoadModel("model/Male03/Male03.FBX"); 
	mMoveDefScale = 0.07;
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
	mTransform->SetPosition(Eigen::Vector3f(0, -5, 0));}

void TestPBR::OnRender()
{
	if (mModel) {
		mModel->Update(mTimer->mDeltaTime);
		if (mContext->RenderPipe()->BeginFrame()) {
			RenderOperationQueue opQueue;
			mModel->GenRenderOperation(opQueue);
			mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
			mContext->RenderPipe()->EndFrame();
		}
	}
}

#if defined TEST_PBR && TEST_CASE == TEST_PBR
auto reg = AppRegister<TestPBR>("TestPBR: PBR");
#endif