#include "test/framework/test_case.h"
#include "core/mir_config_macros.h"
#include "core/rendersys/render_pipeline.h"

using namespace mir;
using namespace mir::rend;

class TestGLTF : public App
{
protected:
	CoTask<bool> OnInitScene() override;
	void OnInitLight() override {}
	void OnInitCamera() override {}
	void CleanUp() override {
		mModel = nullptr;
		mMtls.clear();
		mDeferredMtl.Reset();
	}
private:
	CoTask<void> UpdateDeferredMtlKeywords(std::string keyword, int value) {
		if (mMtls.empty()) {
			mModel->GetMaterials(mMtls);
			mContext->RenderPipe()->GetDefferedMaterial(mDeferredMtl);
		}

		mDeferredMtl.UpdateKeyword(keyword, value);
		CoAwait mDeferredMtl.CommitKeywords(__LaunchAsync__, *mResMng);
		CoReturn;
	}
	CoTask<void> UpdateMtlsKeywords(std::string keyword, int value) {
		if (mMtls.empty()) {
			mModel->GetMaterials(mMtls);
			mContext->RenderPipe()->GetDefferedMaterial(mDeferredMtl);
		}

		for (auto& mtl : mMtls) {
			mtl.UpdateKeyword(keyword, value);
			CoAwait mtl.CommitKeywords(__LaunchAsync__, *mResMng);
		}
		CoReturn;
	}
	std::vector<res::MaterialInstance> mMtls;
	res::MaterialInstance mDeferredMtl;
private:
	AssimpModelPtr mModel;
	int mRenderingPath = kRenderPathForward;
	bool _USE_IBL = true;
	bool _USE_PUNCTUAL = true;
	int _DEBUG_CHANNEL = 0;
};

CoTask<bool> TestGLTF::OnInitScene()
{
	TIME_PROFILE("testGLTF.OnInitScene");

	CameraPtr camera = mScneMng->CreateCameraNode(kCameraPerspective);
	camera->SetFov(0.9 * boost::math::constants::radian<float>());
	mRenderingPath = mCaseSecondIndex;
	camera->SetRenderingPath((RenderingPath)mRenderingPath);

	auto canvas = mScneMng->CreateGuiCanvasNode();
	auto cmd = [&,this]()->CoTask<void>
	{
		ImGui::Text("current rendering-path: %s", (mRenderingPath == kRenderPathForward ? "forward" : "deferred"));
		ImGui::SameLine();
		if (ImGui::Button("switch")) {
			mRenderingPath = (mRenderingPath + 1) % 2;
			camera->SetRenderingPath((RenderingPath)mRenderingPath);
		}
		
		if (mRenderingPath == kRenderPathForward) {
		#define MACRO_CHECK_BOX(LABEL, VAR) if (ImGui::Checkbox(LABEL, &_##VAR)) CoAwait UpdateMtlsKeywords(#VAR, _##VAR);
			MACRO_CHECK_BOX("enable IBL", USE_IBL);
			MACRO_CHECK_BOX("enable Punctual", USE_PUNCTUAL);

			const char* items[] = {
				"none", "uv0", "uv1", "normal texture", "geometry normal",
				"geometry tangent", "geometry bitangent", "shading normal", "alpha", "ao",
				"emissive", "brdf diffuse", "brdf specular", "ibl diffuse", "ibl specular",
				"lut", "metallic roughness", "basecolor", "metallic", "perceptual roughness",
				"transmission", "sheen", "clear coat", 
			};

			int item_macros[] = {
				DEBUG_CHANNEL_NONE, DEBUG_CHANNEL_UV_0, DEBUG_CHANNEL_UV_1, DEBUG_CHANNEL_NORMAL_TEXTURE, DEBUG_CHANNEL_GEOMETRY_NORMAL,
				DEBUG_CHANNEL_GEOMETRY_TANGENT, DEBUG_CHANNEL_GEOMETRY_BITANGENT, DEBUG_CHANNEL_SHADING_NORMAL, DEBUG_CHANNEL_ALPHA, DEBUG_CHANNEL_OCCLUSION,
				DEBUG_CHANNEL_EMISSIVE, DEBUG_CHANNEL_BRDF_DIFFUSE, DEBUG_CHANNEL_BRDF_SPECULAR, DEBUG_CHANNEL_IBL_DIFFUSE, DEBUG_CHANNEL_IBL_SPECULAR,
				DEBUG_CHANNEL_LUT, DEBUG_CHANNEL_METTALIC_ROUGHNESS, DEBUG_CHANNEL_BASECOLOR, DEBUG_CHANNEL_METTALIC, DEBUG_CHANNEL_PERCEPTUAL_ROUGHNESS,
				DEBUG_CHANNEL_TRANSMISSION_VOLUME, DEBUG_CHANNEL_SHEEN, DEBUG_CHANNEL_CLEARCOAT, DEBUG_CHANNEL_GBUFFER_POS, DEBUG_CHANNEL_GBUFFER_NORMAL,
				DEBUG_CHANNEL_GBUFFER_ALBEDO
			};
			if (ImGui::ListBox("debug channel", &_DEBUG_CHANNEL, items, IM_ARRAYSIZE(items), 8)) {
				int debug_channel = item_macros[_DEBUG_CHANNEL];
				CoAwait UpdateMtlsKeywords("DEBUG_CHANNEL", debug_channel);
			}
		}
		else {
			const char* items[] = {
				"none", "gbuffer-pos", "gbuffer-normal", "gbuffer-albedo"
			};

			int item_macros[] = {
				DEBUG_CHANNEL_NONE, DEBUG_CHANNEL_GBUFFER_POS, DEBUG_CHANNEL_GBUFFER_NORMAL, DEBUG_CHANNEL_GBUFFER_ALBEDO
			};

			if (ImGui::ListBox("debug channel", &_DEBUG_CHANNEL, items, IM_ARRAYSIZE(items), 8)) {
				int debug_channel = item_macros[_DEBUG_CHANNEL];
				CoAwait UpdateDeferredMtlKeywords("DEBUG_CHANNEL", debug_channel);
			}
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		CoReturn;
	};
	mGuiMng->AddCommand(cmd);

	test1::res::model model;
	{
		if (mCaseIndex == 0) {
			auto transform = camera->GetTransform();
			camera->SetClippingPlane(Eigen::Vector2f(0.001, 2.0));

			Eigen::Vector3f eyePos = mir::math::point::ToLeftHand(Eigen::Vector3f(
				-0.0169006381,
				0.0253599286,
				0.0302319955));
			camera->SetLookAt(eyePos, eyePos + Eigen::Vector3f(0, 0, 1));

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

		std::string modelNameArr[] = { "toycar" };
		int caseIndex = mCaseIndex;
		mTransform = CoAwait model.Init(modelNameArr[0], mModel);
	}
	CoReturn true;
}

auto reg = AppRegister<TestGLTF>("test_gltf");
