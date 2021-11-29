#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"

using namespace mir;

class TestBloom : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModelPtr mModel = nullptr;
};

void TestBloom::OnPostInitDevice()
{
	mContext->SceneMng()->RemoveAllCameras();
	auto camera = mContext->SceneMng()->AddPerspectiveCamera(Eigen::Vector3f(0,0,-10), 300, 45);
	
	auto rendFac = mContext->RenderableFac();
	camera->SetSkyBox(rendFac->CreateSkybox("model/uffizi_cross.dds"));
	camera->AddPostProcessEffect(rendFac->CreatePostProcessEffect(E_MAT_POSTPROC_BLOOM, *camera));

	mModel = mContext->RenderableFac()->CreateAssimpModel(mTransform, E_MAT_MODEL); 
	mModel->LoadModel("model/Spaceship/Spaceship.fbx"); 
	mMoveDefScale = 0.01;
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
}

void TestBloom::OnRender()
{
	if (mModel) {
		mModel->Update(mTimer->mDeltaTime);
		mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
	}
}

auto reg = AppRegister<TestBloom>("test_bloom");