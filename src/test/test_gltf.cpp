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

inline MaterialLoadParamBuilder GetMatName(int secondIndex) {
	MaterialLoadParamBuilder mlpb(MAT_MODEL);
	mlpb["PBR_MODE"] = 2;
	return mlpb;
}

void ReadAndPrintBinary(const std::string& path) {

}

void TestGLTF::OnPostInitDevice()
{
	mir::CameraPtr camera = mScneMng->AddPerspectiveCamera(test1::cam::Eye(mWinCenter));

	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1:
	case 2: {
		auto dir_light = mScneMng->AddDirectLight();
		dir_light->SetDirection(Eigen::Vector3f(-0.7399, -0.6428, 0.1983));

		camera->SetLookAt(Eigen::Vector3f(0, 0, -5), Eigen::Vector3f::Zero());
		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		skyMat["CubeMapIsRightHandness"] = TRUE;
		camera->SetSkyBox(mRendFac->CreateSkybox(test1::res::Sky(2), skyMat));

		MaterialLoadParamBuilder modelMat = GetMatName(mCaseSecondIndex);
		modelMat["CubeMapIsRightHandness"] = TRUE;
		mModel = mRendFac->CreateAssimpModel(modelMat);
		std::string modelNameArr[] = { "damaged-helmet", "toycar", "box-space" };
		int caseIndex = mCaseIndex;
		mTransform = model.Init(modelNameArr[caseIndex], mModel);
	}break;
	case 3: {
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
