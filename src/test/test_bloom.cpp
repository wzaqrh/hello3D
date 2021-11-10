#include "test/test_case.h"

#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestBloom : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
private:
	AssimpModel* mModel = nullptr;
};

void TestBloom::OnPostInitDevice()
{
	mContext->SceneMng()->SetPerspectiveCamera(XMFLOAT3(0,0,-10), 300, 45);
	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("images\\uffizi_cross.dds"));
	mContext->SceneMng()->GetDefCamera()->AddPostProcessEffect(
		mContext->RenderableFac()->CreatePostProcessEffect(E_MAT_POSTPROC_BLOOM, *mContext->SceneMng()->GetDefCamera()));

	mModel = new AssimpModel(*mContext->RenderSys(), *mContext->MaterialFac(), mMove, E_MAT_MODEL);
	gModelPath = "Spaceship\\"; if (mModel) mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
}

void TestBloom::OnRender()
{
	if (mModel) {
		mModel->Update(mTimer->mDeltaTime);
		mContext->RenderPipe()->Draw(*mModel);
	}
}

#if defined TEST_BLOOM && TEST_CASE == TEST_BLOOM
auto reg = AppRegister<TestBloom>("TestBloom: PostProcess Bloom");
#endif