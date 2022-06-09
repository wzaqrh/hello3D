#include "test/framework/test_case.h"

using namespace mir;
using namespace mir::rend;

class TestGLTF : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
private:
	AssimpModelPtr mModel;
};

CoTask<bool> TestGLTF::OnInitScene()
{
	TIME_PROFILE("testGLTF.OnInitScene");

	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);
	camera->SetFov(0.9 * boost::math::constants::radian<float>());
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1:
	case 2:
	case 3: 
	case 4: {
		if (mCaseIndex == 1) {
			auto transform = camera->GetTransform();
			camera->SetClippingPlane(Eigen::Vector2f(0.001, 2.0));

			Eigen::Vector3f eyePos = mir::math::point::ToLeftHand(Eigen::Vector3f(
				-0.0169006381,
				0.0253599286,
				0.0302319955));
			camera->SetLookAt(eyePos, eyePos + Eigen::Vector3f(0,0,1));

			transform->SetRotation(mir::math::quat::ToLeftHand(Eigen::Quaternionf(
				0.8971969,
				-0.330993533,
				-0.274300218,
				-0.101194724)));
			mControlCamera = false;
		}
		else {
			camera->SetLookAt(Eigen::Vector3f(0, 0, -5), Eigen::Vector3f::Zero());
		}

		auto dir_light = mScneMng->CreateLightNode<DirectLight>();
		dir_light->SetColor(Eigen::Vector3f::Zero());
		dir_light->SetLookAt(Eigen::Vector3f(-0.5, 0.707, -0.5), Eigen::Vector3f::Zero());
		
		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		skyMat["LIGHTING_MODE"] = 2;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(), skyMat));

		MaterialLoadParamBuilder modelMat = MAT_MODEL;
		modelMat["LIGHTING_MODE"] = 2;
		mModel = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));

		std::string modelNameArr[] = { "damaged-helmet", "toycar", "box-space", "BoomBox", "Box" };
		int caseIndex = mCaseIndex;
		mTransform = CoAwait model.Init(modelNameArr[caseIndex], mModel);
	}break;
	case 10: {
		auto pt_light = mScneMng->CreateLightNode<PointLight>();
		pt_light->SetPosition(Eigen::Vector3f(0, 15, -5));
		pt_light->SetAttenuation(0.001);
	}break;
	default: {
		auto dir_light = mScneMng->CreateLightNode<DirectLight>();
		dir_light->SetDirection(Eigen::Vector3f(0, 0, 1));
	}break;
	}
	CoReturn true;
}

auto reg = AppRegister<TestGLTF>("test_gltf");
