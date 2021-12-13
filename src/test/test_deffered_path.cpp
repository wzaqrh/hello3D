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
