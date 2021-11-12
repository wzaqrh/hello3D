#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestSpecSkybox : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnInitLight() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};

void TestSpecSkybox::OnInitLight()
{
#if 0
	auto light1 = mRenderSys->mPointLights[0];
	light1->SetPosition(20, 0, -20);
	light1->SetAttenuation(1.0, 0.1, 0);
	light1->SetSpecularPower(60);
	light1->SetSpecularColor(0, 0, 0, 0);
	light1->SetDiffuseColor(0, 0, 0, 0);
#endif
	//auto light2 = mRenderSys->AddSpotLight();
	////light2->SetDiffuseColor(0, 0, 0, 0);
	////light2->SetSpecularColor(1, 0, 0, 1);
	//light2->SetDirection(0, 0, 1);
	//light2->SetPosition(0, 0, -10);
	//light2->SetAttenuation(1.0, 0.1, 0);
	//light2->SetAngle(3.14*15/180);

	auto light2 = mContext->SceneMng()->AddDirectLight();
	light2->SetDirection(0, 0, 1);
}

void TestSpecSkybox::OnPostInitDevice()
{
	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("images\\uffizi_cross.dds"));
	
	mModel = new AssimpModel(*mContext->RenderSys(), *mContext->MaterialFac(), mTransform, E_MAT_MODEL);
	gModelPath = "Spaceship\\"; 
	mModel->LoadModel(MakeModelPath("Spaceship.fbx")); 
	mMoveDefScale = 0.01;
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
}

void TestSpecSkybox::OnRender()
{
	if (mModel) {
		mModel->Update(mTimer->mDeltaTime);
		mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
	}
}

#if defined TEST_SPEC_SKYBOX && TEST_CASE == TEST_SPEC_SKYBOX
auto reg = AppRegister<TestSpecSkybox>("Skybox");
#endif