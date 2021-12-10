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
	auto camera = mContext->SceneMng()->AddPerspectiveCamera(Eigen::Vector3f(0,0,-10), Eigen::Vector3f(0.01, 300, 45));
	
	auto mRendFac = mContext->RenderableFac();
	camera->SetSkyBox(mRendFac->CreateSkybox("model/uffizi_cross.dds"));
	camera->AddPostProcessEffect(mRendFac->CreatePostProcessEffect(E_MAT_POSTPROC_BLOOM, *camera));

	mModel = mContext->RenderableFac()->CreateAssimpModel(E_MAT_MODEL); 
	mModel->LoadModel("model/Spaceship/Spaceship.fbx"); 
	mTransform = mModel->GetTransform();
	#define SCALE_BASE 0.01
	mTransform->SetScale(Eigen::Vector3f(SCALE_BASE, SCALE_BASE, SCALE_BASE));
}

void TestBloom::OnRender()
{
	if (mModel) {
		mModel->Update(mTimer->mDeltaTime);
		mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
	}
}

auto reg = AppRegister<TestBloom>("test_bloom");