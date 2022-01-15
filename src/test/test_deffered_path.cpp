#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;
using namespace mir::renderable;

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
0延迟,1正向：透视相机 平行光+电光 对比观察飞机
2延迟,3正向: 透视相机 正向平行光  对比观察飞机
*/

void TestDefferedPath::OnPostInitDevice()
{
	CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));
	camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky()));
	camera->SetRenderingPath(RenderingPath((mCaseIndex+1)&1));

	switch (mCaseIndex) {
	case 0:
	case 1:{
		mScneMng->AddDirectLight();
		mScneMng->AddPointLight()->SetPosition(test1::vec::PosLight());

		mModel = mRendFac->CreateAssimpModel(MAT_MODEL);
		mTransform = test1::res::model_sship::Init(mModel, mWinCenter);
	}break;
	case 2:
	case 3:{
		mScneMng->AddDirectLight();

		mModel = mRendFac->CreateAssimpModel(MAT_MODEL);
		mTransform = test1::res::model_sship::Init(mModel, mWinCenter);
	}break;
	default:
		break;
	}
}

void TestDefferedPath::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestDefferedPath>("test_deffered_path");
