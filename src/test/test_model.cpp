#include "test/test_case.h"
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestModel : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	int mDrawFlag = 0;
	AssimpModelPtr mModel = nullptr;
};

void TestModel::OnPostInitDevice()
{
	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("model/uffizi_cross.dds"));

	mModel = mContext->RenderableFac()->CreateAssimpModel(mTransform, E_MAT_MODEL);
	mModel->LoadModel("model/Male03/Male02.FBX", R"({"ext":"png","dir":"model/Male03/"})"); 
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
