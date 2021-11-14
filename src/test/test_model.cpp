#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestModel : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	int mDrawFlag = 0;
	AssimpModel* mModel = nullptr;
};

void TestModel::OnPostInitDevice()
{
	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("images\\uffizi_cross.dds"));

	mModel = new AssimpModel(*mContext->RenderSys(), *mContext->MaterialFac(), mTransform, E_MAT_MODEL);
	//gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mTransform->SetDefScale(0.01); mModel->PlayAnim(0);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); 
	//gModelPath = "handgun\\"; mModel->LoadModel(MakeModelPath("handgun.fbx")); mTransform->SetDefScale(0.01);
	SetModelPath("Male03\\"); mModel->LoadModel(MakeModelPath("Male02.FBX")); 
	mMoveDefScale = 0.07; 
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
	mTransform->SetPosition(Eigen::Vector3f(0, -5, 0));
}

void TestModel::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

#if defined TEST_MODEL && TEST_CASE == TEST_MODEL
auto reg = AppRegister<TestModel>("TestModel: Assimp Model");
#endif
