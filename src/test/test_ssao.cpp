#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/cube.h"
#include "core/renderable/post_process.h"

using namespace mir;
using namespace mir::rend;

class TestSSAO : public App
{
protected:
	CoTask<bool> OnInitScene() override;
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

CoTask<bool> TestSSAO::OnInitScene()
{
	TIME_PROFILE("testSSAO.OnInitScene");

	CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
	camera->SetFov(0.9 * boost::math::constants::radian<float>());
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	test1::res::model model;
	switch (mCaseIndex) {
	case 0: {
		camera->SetLookAt(Eigen::Vector3f(0, 2, 0), Eigen::Vector3f::Zero());

		auto dir_light = mScneMng->CreateAddLightNode<DirectLight>();
		dir_light->SetDirection(Eigen::Vector3f(-0.498, 0.71, -0.498));

		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		skyMat["CubeMapIsRightHandness"] = TRUE;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(2), skyMat));

		MaterialLoadParamBuilder modelMat = GetMatName(mCaseSecondIndex);
		modelMat["CubeMapIsRightHandness"] = TRUE;
		mModel = mScneMng->AddRendNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		mTransform = CoAwait model.Init("dragon", mModel);
		mTransform->SetScale(Eigen::Vector3f(0.005, 0.005, 0.005));

		auto floor = mScneMng->AddRendNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		auto floorModel = CoAwait model.Init("floor", floor);
		floorModel->SetEulerAngles(Eigen::Vector3f(3.14 * 0.5, 0, 0));
		floorModel->SetScale(Eigen::Vector3f(0.1, 0.1, 0.1));
	#if 1
		auto effect = CoAwait PostProcessFactory(mRendFac).CreateGaussianBlur(2);
		//camera->AddPostProcessEffect(effect);
		camera->SetRenderingPath(kRenderPathDeffered);
	#endif
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestSSAO>("test_ssao");
