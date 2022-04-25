#include "test/test_case.h"
#include "test/app.h"
#include "core/renderable/sprite.h"
#include "core/renderable/cube.h"
#include "test/unit_test/unit_test.h"

using namespace mir;
using namespace mir::rend;

class TestShadowMap : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};

inline MaterialLoadParamBuilder GetMatName(int secondIndex) {
	MaterialLoadParamBuilder mlpb(MAT_MODEL);
	mlpb["PBR_MODE"] = 2;
	return mlpb;
}

CoTask<bool> TestShadowMap::OnInitScene()
{
	//SetPPU(1);
	TIME_PROFILE("testSSAO.OnInitScene");

	CameraPtr camera = mScneMng->CreateAddCameraNode(kCameraPerspective, test1::cam::Eye(mWinCenter));
	camera->SetFov(45.0);
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1:{
		camera->SetLookAt(Eigen::Vector3f(-0.644995f, 0.614183f, -0.660632f), Eigen::Vector3f::Zero());

		auto dir_light = mScneMng->CreateAddLightNode<SpotLight>();
		dir_light->SetLookAt(Eigen::Vector3f(3.57088f, 6.989f, -9.19698f), Eigen::Vector3f::Zero());
		dir_light->SetLightRadius(1.0);

		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		skyMat["CubeMapIsRightHandness"] = TRUE;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(2), skyMat));

		MaterialLoadParamBuilder modelMat = GetMatName(mCaseSecondIndex);
		modelMat["CubeMapIsRightHandness"] = TRUE;

		{
			auto floor = mScneMng->AddRendNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
			auto floorModel = CoAwait model.Init("floor", floor);
			floorModel->SetEulerAngles(Eigen::Vector3f(3.14 * 0.5, 0, 0));
			floorModel->SetScale(Eigen::Vector3f(0.1, 0.1, 0.1));
			if (mCaseIndex == 1) {
				floorModel->SetPosition(Eigen::Vector3f(0, -0.4, 0));
			}
			floor->SetCastShadow(false);
		}

		{
			mModel = mScneMng->AddRendNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
			if (mCaseIndex == 1) {
				mTransform = CoAwait model.Init("buddha", mModel);
				mTransform->SetEulerAngles(Eigen::Vector3f(0, 3.14, 0));
			}
			else {
				mTransform = CoAwait model.Init("armadillo", mModel);
				mTransform->SetScale(Eigen::Vector3f(0.005, 0.005, 0.005));
			}
		}

		//camera->SetRenderingPath(mir::kRenderPathDeffered);
	}break;
	default:
		break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestShadowMap>("test_shadow_map");