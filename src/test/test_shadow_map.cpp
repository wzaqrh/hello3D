#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/base/transform.h"
#include "test/unit_test/unit_test.h"

using namespace mir;
using namespace mir::renderable;

class TestShadowMap : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModelRock, mModelPlanet, mModelFloor;
	CubePtr mCube0, mCube1;
};
/*mCaseIndex
0：透视相机, 相机朝下 方向光朝下偏前 飞机投影到地板
1: 透视相机, 相机朝前 方向光朝前偏上 飞机投影到地板
*/

void TestShadowMap::OnPostInitDevice()
{
	SetPPU(1);

	if (1) {
		mModelFloor = mRendFac->CreateAssimpModel(MAT_MODEL);
		mModelFloor->LoadModel(test1::res::model_floor::Path(), test1::res::model_floor::Rd());
		mModelFloor->GetTransform()->SetScale(test1::res::model_floor::Scale() * 10);
	}

	if (1) {
		mModelRock = mRendFac->CreateAssimpModel(MAT_MODEL);
		mModelRock->LoadModel(test1::res::model_sship::Path(), test1::res::model_sship::Rd());
		mTransform = mModelRock->GetTransform();
		mTransform->SetScale(test1::res::model_sship::Scale() * 3);
	}

	switch (mCaseIndex) {
	case 0: {
		mControlCamera = false;

		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(0, -3, -1));
		auto camera = mScneMng->AddPerspectiveCamera(Eigen::Vector3f(0, 10, 0));
		camera->SetForward(mir::math::vec::Down());
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));

		mModelFloor->GetTransform()->SetPosition(test1::res::model_floor::Pos() + Eigen::Vector3f(0, -100, 0));
		mModelFloor->GetTransform()->Rotate(Eigen::Vector3f(3.14, 0, 0));
		mModelRock->GetTransform()->SetPosition(test1::res::model_sship::Pos() + Eigen::Vector3f(0, -30, 0));
	}break;
	case 1: {
		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(0, 1, 3));
		auto camera = mScneMng->AddPerspectiveCamera(Eigen::Vector3f(0, 0, -10));
		camera->SetForward(mir::math::vec::Forward());

		mModelFloor->GetTransform()->SetPosition(test1::res::model_floor::Pos() + Eigen::Vector3f(0, 0, 100));
		mModelFloor->GetTransform()->Rotate(Eigen::Vector3f(3.14 / 2, 0, 0));
		mTransform->SetPosition(test1::res::model_sship::Pos() + Eigen::Vector3f(0, 0, 30));
	}break;
	default:
		break;
	}
}

void TestShadowMap::OnRender()
{
	if (mContext->RenderPipe()->BeginFrame()) {
		if (mModelFloor) mModelFloor->Update(mTimer->mDeltaTime);
		if (mModelRock) mModelRock->Update(mTimer->mDeltaTime);
		if (mModelPlanet) mModelPlanet->Update(mTimer->mDeltaTime);

		RenderOperationQueue opQueue;
		if (mCube0) mCube0->GenRenderOperation(opQueue);
		if (mCube1) mCube1->GenRenderOperation(opQueue);
		if (mModelFloor) mModelFloor->GenRenderOperation(opQueue);
		if (mModelRock) mModelRock->GenRenderOperation(opQueue);
		if (mModelPlanet) mModelPlanet->GenRenderOperation(opQueue);

		mContext->RenderPipe()->Render(opQueue, *mContext->SceneMng());
		mContext->RenderPipe()->EndFrame();
	}
}

auto reg = AppRegister<TestShadowMap>("test_shadow_map");