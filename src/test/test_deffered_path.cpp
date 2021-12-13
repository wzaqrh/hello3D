#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestDefferedPath : public App
{
protected:
	void OnRender() override;
	void OnPostInitDevice() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};
/*mCaseIndex
0,1：透视相机 观察到模型：传奇战士 + 天空
2,3: 透视相机 观察到模型：飞机
4,5：透视相机 观察到模型：石头（在右上角）

6,7：正交相机 观察到模型：传奇战士 + 天空
8,9: 正交相机 观察到模型：飞机
10,11：正交相机 观察到模型：石头（在右上角）
*/

void TestDefferedPath::OnPostInitDevice()
{
	if (mCaseIndex == 0) {
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}
	else {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(mWinCenter + Eigen::Vector3f(0, 5, -5));

		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(25, 0, 5));
	}

	mir::CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter), test1::cam::NearFarFov());

	mModel = mRendFac->CreateAssimpModel(MAT_MODEL);
	mTransform = test1::res::model_rock::Init(mModel, mWinCenter);
}

void TestDefferedPath::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestDefferedPath>("test_deffered_path");
