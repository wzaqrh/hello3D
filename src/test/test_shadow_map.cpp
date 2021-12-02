#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/base/transform.h"
#include "test/unit_test/unit_test.h"

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
	AssimpModelPtr mModel1, mModel2;
	CubePtr mCube;
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

#endif
}

#define USE_RENDER_TEXTURE
#define SCALE_BASE 100
void TestShadowMap::OnPostInitDevice()
{
	auto sceneMng = mContext->SceneMng();
	auto rendFac = mContext->RenderableFac();
	auto size = mContext->ResourceMng()->WinSize() / 2;

	sceneMng->RemoveAllLights();
	auto dir_light = sceneMng->AddDirectLight();
	dir_light->SetDirection(Eigen::Vector3f(-1, -1, 3));

	sceneMng->RemoveAllCameras();
	auto camera = sceneMng->AddOthogonalCamera(Eigen::Vector3f(0, 0, -1500), 3000); 
	camera->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, 0));

	test::CompareLightCameraByViewProjection(*dir_light, *camera, {});

	if (1)
	{
		const int SizeInf = 10000;
		mCube = mContext->RenderableFac()->CreateCube(Eigen::Vector3f(0, 0, 200), Eigen::Vector3f(SizeInf, SizeInf, 2));
	}

	if (1)
	{
		mModel1 = rendFac->CreateAssimpModel(nullptr, E_MAT_MODEL);
		mModel1->LoadModel("model/rock/rock.obj", R"({"dir":"model/rock/"})");
		mModel1->GetTransform()->SetScale(Eigen::Vector3f(SCALE_BASE, SCALE_BASE, SCALE_BASE));
		mModel1->GetTransform()->SetPosition(Eigen::Vector3f(100, 100, 10));
		mTransform = mModel1->GetTransform();
	}

	if (1)
	{
		mModel2 = rendFac->CreateAssimpModel(nullptr, E_MAT_MODEL);
		mModel2->LoadModel("model/planet/planet.obj", R"({"dir":"model/planet/"})");
		mMoveDefScale = SCALE_BASE * 0.2;
		mModel2->GetTransform()->SetScale(Eigen::Vector3f(mMoveDefScale, mMoveDefScale, mMoveDefScale));
		mModel2->GetTransform()->SetPosition(Eigen::Vector3f(100, 100, 10) + Eigen::Vector3f(100, 100, -300));
	}
}

void TestShadowMap::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		if (mModel1) mModel1->Update(mTimer->mDeltaTime);
		if (mModel2) mModel2->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		if (mCube) mCube->GenRenderOperation(opQueue);
		if (mModel1) mModel1->GenRenderOperation(opQueue);
		if (mModel2) mModel2->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestShadowMap>("test_shadow_map");