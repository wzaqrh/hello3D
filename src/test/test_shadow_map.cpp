#include "test/test_case.h"

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
	cbPointLightPtr mLight;
};

void TestShadowMap::OnInitLight()
{
	mLight = mContext->SceneMng()->AddPointLight();//1, -1, 1
	float ddd = 10;
	mLight->SetPosition(ddd, ddd, -ddd);
	mLight->SetAttenuation(1, 0.001, 0);
	//mLight->SetPosition(0, 0, -150);
}

#define USE_RENDER_TEXTURE
#define SCALE_BASE 0.01
void TestShadowMap::OnPostInitDevice()
{
	mContext->SceneMng()->RemoveAllCameras();
	mContext->SceneMng()->AddPerspectiveCamera(Eigen::Vector3f(0,0,-30), 300, 45);
	mContext->SceneMng()->GetDefCamera()->SetSkyBox(
		mContext->RenderableFac()->CreateSkybox("images\\uffizi_cross.dds"));

	float dd = 3;
	mMoveDefScale = SCALE_BASE * 0.2;
	mTransform->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
	mTransform->SetPosition(Eigen::Vector3f(0, 0, -dd));


	//std::string matName = E_MAT_MODEL_PBR;
	std::string matName = E_MAT_MODEL;

	auto move1 = std::make_shared<Transform>();
	move1->SetScale(Eigen::Vector3f(SCALE_BASE, SCALE_BASE, SCALE_BASE));
	mModel1 = new AssimpModel(*mContext->RenderSys(), *mContext->MaterialFac(), move1, matName);
	gModelPath = "Spaceship\\"; mModel1->LoadModel(MakeModelPath("Spaceship.fbx"));

	mModel2 = new AssimpModel(*mContext->RenderSys(), *mContext->MaterialFac(), mTransform, matName);
	gModelPath = "Spaceship\\"; mModel2->LoadModel(MakeModelPath("Spaceship.fbx"));
}

void TestShadowMap::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		mModel1->Update(mTimer->mDeltaTime);
		mModel2->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		mModel1->GenRenderOperation(opQueue);
		mModel2->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

#if defined TEST_SHADOW_MAP && TEST_CASE == TEST_SHADOW_MAP
auto reg = AppRegister<TestShadowMap>("Lesson7: ShadowMap");
#endif