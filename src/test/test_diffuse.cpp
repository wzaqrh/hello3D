#include "test/test_case.h"
#if defined TEST_DIFFUSE && TEST_CASE == TEST_DIFFUSE
#include "test/app.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

using namespace mir;

class TestDiffuse : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModel* mModel = nullptr;
};

void TestDiffuse::OnInitLight()
{
	auto light = mContext->SceneMng()->AddPointLight();
	light->SetDiffuseColor(1, 1, 1, 1);
	light->SetPosition(0, 0, -200);
}

void TestDiffuse::OnPostInitDevice()
{
	mModel = new AssimpModel(*mContext->RenderSys(), *mContext->MaterialFac(), mMove, E_MAT_MODEL);
	gModelPath = "Spaceship\\"; mModel->LoadModel(MakeModelPath("Spaceship.fbx")); mMove->SetDefScale(0.01);
	//gModelPath = "Normal\\"; mModel->LoadModel(MakeModelPath("Deer.fbx")); mScale = 0.05;
	//mModel->PlayAnim(0);
}

void TestDiffuse::OnRender()
{
	if (mModel) {
		mModel->Update(mTimer->mDeltaTime);
		mContext->RenderPipe()->Draw(*mModel);
	}
}

auto reg = AppRegister<TestDiffuse>("TestDiffuse: Diffuse Light");
#endif