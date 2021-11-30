#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"

using namespace mir;

struct cbShadowMap
{
	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;
};

class TestShadowMap : public App
{
protected:
	virtual void OnRender() override;
	virtual void OnPostInitDevice() override;
	virtual void OnInitLight() override;
private:
	AssimpModelPtr mModel1, mModel2 = nullptr;
};

void TestShadowMap::OnInitLight()
{
#if 0
	mLight = mContext->SceneMng()->AddPointLight();//1, -1, 1
	float ddd = 10;
	mLight->SetPosition(Eigen::Vector3f(ddd, ddd, -ddd));
	mLight->SetAttenuation(0.001);
	//mLight->SetPosition(0, 0, -150);
#else
	mContext->SceneMng()->RemoveAllLights();
	auto dir_light = mContext->SceneMng()->AddDirectLight();
	dir_light->SetDirection(Eigen::Vector3f(-1, -1, 1));
	//dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
#endif
}

#define USE_RENDER_TEXTURE
#define SCALE_BASE 1
void TestShadowMap::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();

	sceneMng->RemoveAllCameras();
	sceneMng->AddPerspectiveCamera(Eigen::Vector3f(0,0,-1500), 3000, 30);
	//sceneMng->GetDefCamera()->SetSkyBox(rendFac->CreateSkybox("model/uffizi_cross.dds"));

	{
		mModel1 = rendFac->CreateAssimpModel(nullptr, E_MAT_MODEL);
		mModel1->LoadModel("model/Spaceship/Spaceship.fbx", R"({"dir":"model/Spaceship/"})");
		mModel1->GetTransform()->SetScale(Eigen::Vector3f(SCALE_BASE, SCALE_BASE, SCALE_BASE));
	}

	if (1)
	{
		mModel2 = rendFac->CreateAssimpModel(nullptr, E_MAT_MODEL);
		mModel2->LoadModel("model/Spaceship/Spaceship.fbx", R"({"dir":"model/Spaceship/"})");
		mMoveDefScale = SCALE_BASE * 0.2;
		mModel2->GetTransform()->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
		mModel2->GetTransform()->SetPosition(Eigen::Vector3f(350, 350, -350));
	}
}

void TestShadowMap::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		mModel1->Update(mTimer->mDeltaTime);
		if (mModel2) mModel2->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		mModel1->GenRenderOperation(opQueue);
		if (mModel2) mModel2->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestShadowMap>("test_shadow_map");