#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;
using namespace mir::rend;

class TestLight : public App
{
protected:
	void OnRender() override;
	CoTask<void> OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};
/*mCaseIndex
0：透视相机 上前侧点光		观察到飞机高光偏上
1: 透视相机 左前侧点光		观察到飞机高光偏左
2：透视相机 右侧点光			观察到飞机高光在右, 左边几乎全黑
3：透视相机 下右前侧远点光		观察到飞机高光偏右下但非常暗

4: 正交相机
5: 正交相机
6: 正交相机
7: 正交相机
*/

CoTask<void> TestLight::OnPostInitDevice()
{
	constexpr int CaseCountMod = 4;
	int caseIndex = mCaseIndex % CaseCountMod;
	int cameraType = mCaseIndex / CaseCountMod;
	//SetPPU(1);

	switch (caseIndex) {
	case 0: {
		mScneMng->AddPointLight()->SetPosition(Eigen::Vector3f(0, 10, -5));
	}break;
	case 1: {
		mScneMng->AddPointLight()->SetPosition(Eigen::Vector3f(-10, 0, -5));
	}break;
	case 2: {
		mScneMng->AddPointLight()->SetPosition(Eigen::Vector3f(10, 0, 0));
	}break;
	case 3: {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(15, -15, -10));
		pt_light->SetAttenuation(0.01);
	}break;
	default: {
		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(25, 0, 5));
		mScneMng->AddDirectLight()->SetDirection(Eigen::Vector3f(0, 0, 1));
	}break;
	}

	CameraPtr camera = mScneMng->AddCameraByType((CameraType)cameraType, test1::cam::Eye(mWinCenter));
	camera->SetSkyBox(CoAwait mRendFac->CreateSkybox(test1::res::Sky()));

	mModel = CoAwait mRendFac->CreateAssimpModel(MAT_MODEL);
	mTransform = CoAwait test1::res::model().Init("spaceship", mModel);
	CoReturnVoid;
}

void TestLight::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestLight>("test_light");
