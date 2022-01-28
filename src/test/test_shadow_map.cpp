#include "test/test_case.h"
#include "test/app.h"
#include "core/scene/scene_manager.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "core/base/transform.h"
#include "test/unit_test/unit_test.h"

using namespace mir;
using namespace mir::rend;

class TestShadowMap : public App
{
protected:
	void OnRender() override;
	CoTask<bool> OnPostInitDevice() override;
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

CoTask<bool> TestShadowMap::OnPostInitDevice()
{
	SetPPU(1);

	if (1) {
		mModelFloor = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
		CoAwait test1::res::model().Init("floor", mModelFloor);
	}

	if (1) {
		mModelRock = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
		mTransform = CoAwait test1::res::model().Init("spaceship", mModelRock);
	}

	switch (mCaseIndex) {
	case 0: {
		mControlCamera = false;

		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(0, -3, -1));
		auto camera = mScneMng->AddPerspectiveCamera(Eigen::Vector3f(0, 10, 0));
		camera->SetForward(mir::math::vec::Down());
		camera->SetSkyBox(CoAwait mRendFac->CreateSkybox(test1::res::Sky()));

		mModelFloor->GetTransform()->SetPosition(Eigen::Vector3f(0, -100, 0));
		mModelFloor->GetTransform()->Rotate(Eigen::Vector3f(3.14, 0, 0));
		mModelRock->GetTransform()->SetPosition(Eigen::Vector3f(0, -30, 0));
	}break;
	case 1: {
		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(0, 1, 3));
		auto camera = mScneMng->AddPerspectiveCamera(Eigen::Vector3f(0, 0, -10));
		camera->SetForward(mir::math::vec::Forward());

		mModelFloor->GetTransform()->SetPosition(Eigen::Vector3f(0, 0, 100));
		mModelFloor->GetTransform()->Rotate(Eigen::Vector3f(3.14 / 2, 0, 0));
		mTransform->SetPosition(Eigen::Vector3f(0, 0, 30));
	}break;
	default:
		break;
	}
	CoReturn true;
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