#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestModel : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override;
private:
	int mDrawFlag = 0;
	AssimpModelPtr mModel = nullptr;
};

void TestModel::OnInitLight()
{
	auto pt_light =mContext->SceneMng()->AddPointLight();
	pt_light->SetPosition(Eigen::Vector3f(0, 5, -5));

	auto dir_light = mContext->SceneMng()->AddDirectLight();
	dir_light->SetDirection(Eigen::Vector3f(25, 0, 5));
}

void TestModel::OnPostInitDevice()
{
	//mContext->SceneMng()->GetDefCamera()->SetSkyBox(
	//	mContext->RenderableFac()->CreateSkybox("model/uffizi_cross.dds"));

	if (mCaseIndex == 0 || mCaseIndex == 1) {
		mModel = mContext->RenderableFac()->CreateAssimpModel(mTransform, mCaseIndex == 0 ? E_MAT_MODEL : E_MAT_MODEL_PBR);
		mModel->LoadModel("model/Male03/Male02.FBX", R"({"ext":"png","dir":"model/Male03/"})"); mMoveDefScale = 0.07;
	}
	else {
		mModel = mContext->RenderableFac()->CreateAssimpModel(mTransform, mCaseIndex == 2 ? E_MAT_MODEL : E_MAT_MODEL_PBR);
		mModel->LoadModel("model/Spaceship/Spaceship.fbx", R"({"dir":"model/Spaceship/"})"); mMoveDefScale = 0.01;
	}
	
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
	mTransform->SetPosition(Eigen::Vector3f(0, -5, 0));

	mModel->PlayAnim(0);
}

void TestModel::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestModel>("test_model");
