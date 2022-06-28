#include "test/framework/test_case.h"
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
	void CleanUp() override {
		mGuiDebugChannel.Dispose();
	}
private:
	GuiDebugWindow mGuiDebugChannel;
};

CoTask<bool> TestSSAO::OnInitScene()
{
	TIME_PROFILE("testSSAO.OnInitScene");

	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);
	camera->SetFov(40.0);
	camera->SetRenderingPath((RenderingPath)mCaseSecondIndex);

	mGuiDebugChannel.Init(mContext);

	AssimpModelPtr mModel;
	test1::res::model model;
	switch (mCaseIndex) {
	case 0:
	case 1:{
		camera->SetLookAt(Eigen::Vector3f(0, 1.5, -1.5), Eigen::Vector3f::Zero());

		auto dir_light = mScneMng->CreateLightNode<DirectLight>();
		dir_light->SetDirection(Eigen::Vector3f(-0.498, 0.71, -0.498));

		MaterialLoadParamBuilder skyMat = MAT_SKYBOX;
		camera->SetSkyBox(CoAwait mRendFac->CreateSkyboxT(test1::res::Sky(), skyMat));

		MaterialLoadParamBuilder modelMat = MAT_MODEL;
		mModel = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		mGuiDebugChannel.AddModel(mModel);
		mTransform = CoAwait model.Init("dragon", mModel);
		mTransform->SetScale(Eigen::Vector3f(0.007, 0.007, 0.007));

		auto floor = mScneMng->AddRendAsNode(CoAwait mRendFac->CreateAssimpModelT(modelMat));
		mGuiDebugChannel.AddModel(floor);
		auto floorModel = CoAwait model.Init("floor", floor);
		floorModel->SetEulerAngles(Eigen::Vector3f(3.14 * 0.5, 0, 0));
		floorModel->SetScale(Eigen::Vector3f(0.1, 0.1, 0.1));

		if (mCaseIndex == 0) {
			auto effect = SSAOBuilder(CoAwait PostProcessFactory(mRendFac).CreateSSAO(*camera))
				.SetRadius(0.35)
				.SetAngleBias(30)
				.SetDirNum(16)
				.SetStepNum(8)
				.SetAttenuation(1.0f)
				.SetContrast(1.25)
				.SetSharpness(16)
				.SetBlurRadius(7)
				.Build();
			camera->AddPostProcessEffect(effect);
			camera->SetRenderingPath(mir::kRenderPathDeffered);

			mGuiDebugChannel.AddPostProcessEffect(effect);
		}
	}break;
	default:
		break;
	}

	mGuiDebugChannel.AddRenderBackendSWCmd();
	mGuiDebugChannel.AddSSAOCmd();

	CoReturn true;
}

auto reg = AppRegister<TestSSAO>("test_ssao");
