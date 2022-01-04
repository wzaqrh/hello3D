#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/assimp_model.h"
#include "core/base/transform.h"

using namespace mir;

class TestGLTF : public App
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
0: 
*/

inline MaterialLoadParam GetMatName(int secondIndex) {
	MaterialLoadParamBuilder mlpb(MAT_MODEL);
	mlpb["PBR_MODE"] = 2;
	return mlpb;
}

void TestGLTF::OnPostInitDevice()
{
	mir::CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));

	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1: {
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, -1, 0));

		camera->SetLookAt(Eigen::Vector3f(0, 0, -5), Eigen::Vector3f::Zero());
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky(2)));

		mModel = mRendFac->CreateAssimpModel(GetMatName(mCaseSecondIndex));
		std::string modelNameArr[] = { "damaged-helmet", "toycar" };
		int caseIndex = mCaseIndex;
		mTransform = model.Init(modelNameArr[caseIndex], mModel);
	}break;
	case 2: {
		auto pt_light = mScneMng->AddPointLight();
		pt_light->SetPosition(Eigen::Vector3f(0, 15, -5));
		pt_light->SetAttenuation(0.001);
	}break;
	default: {
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}break;
	}
}

void TestGLTF::OnRender()
{
	mModel->Update(mTimer->mDeltaTime);
	mContext->RenderPipe()->Draw(*mModel, *mContext->SceneMng());
}

auto reg = AppRegister<TestGLTF>("test_gltf");
