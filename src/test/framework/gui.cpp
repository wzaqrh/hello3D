#include "test/framework/gui.h"
#include "core/mir_config_macros.h"
#include "core/rendersys/render_pipeline.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/post_process.h"
#include "core/scene/light.h"
#include "core/scene/camera.h"
#include "core/scene/transform.h"
#include "core/scene/scene_manager.h"

using namespace mir;
using namespace mir::rend;

void GuiDebugWindow::Init(mir::Mir* ctx)
{
	mContext = ctx;
}

void GuiDebugWindow::AddModel(const mir::rend::AssimpModelPtr& model)
{
	mModels.push_back(model);
}

void GuiDebugWindow::AddPostProcessEffect(const mir::rend::PostProcessPtr& effect)
{
	mEffects.push_back(effect);
}

void GuiDebugWindow::Dispose()
{
	mModels.clear();
	mMtls.clear();
	mDeferredMtl.Reset();
}

CoTask<void> GuiDebugWindow::UpdateDeferredMtlKeywords(std::string keyword, int value)
{
	if (mMtls.empty()) {
		for (auto& m : mModels) {
			m->GetMaterials(mMtls);
		}
		mContext->RenderPipe()->GetDefferedMaterial(mDeferredMtl);
	}

	mDeferredMtl.UpdateKeyword(keyword, value);
	CoAwait mDeferredMtl.CommitKeywords(__LaunchAsync__, *mContext->ResourceMng());
	CoReturn;
}

CoTask<void> GuiDebugWindow::UpdateMtlsKeywords(std::string keyword, int value)
{
	if (mMtls.empty()) {
		for (auto& m : mModels) {
			m->GetMaterials(mMtls);
		}
		mContext->RenderPipe()->GetDefferedMaterial(mDeferredMtl);
	}

	for (auto& mtl : mMtls) {
		mtl.UpdateKeyword(keyword, value);
		CoAwait mtl.CommitKeywords(__LaunchAsync__, *mContext->ResourceMng());
	}
	CoReturn;
}

CoTask<void> GuiDebugWindow::UpdateEffectMtlsKeywords(std::string keyword, int value)
{
	COROUTINE_VARIABLES_2(keyword, value);

	if (mEffectMtls.empty()) {
		for (auto& m : mEffects) {
			m->GetMaterials(mEffectMtls);
		}
	}

	for (auto& mtl : mEffectMtls) {
		mtl.UpdateKeyword(keyword, value);
		CoAwait mtl.CommitKeywords(__LaunchAsync__, *mContext->ResourceMng());
	}
	CoReturn;
}

void GuiDebugWindow::AddAllCmds() 
{
	AddRenderBackendSWCmd();
	AddRenderingPathSWCmd();
	AddIBLPunctualSWCmd();
	AddDebugChannelCmd();
}

int ReadAppSetting(const std::string& key0, const std::string& key1, int defValue);
void WriteAppSetting(const std::string& key0, const std::string& key1, int value);

void GuiDebugWindow::AddRenderBackendSWCmd()
{
	mIsOpenglBackend = ReadAppSetting("Rendering Backend", "IsOpengl", TRUE);

	auto sceneMng = mContext->SceneMng();
	auto guiMng = mContext->GuiMng();
	auto camera = sceneMng->GetDefCamera();

	auto canvas = sceneMng->CreateGuiCanvasNode();
	auto cmd = [=]()->CoTask<void> {
		ImGui::Text("current rendering-backend: %s", (mIsOpenglBackend ? "opengl 460" : "directx 11"));
		ImGui::SameLine();
		if (ImGui::Button("switch")) {
			int newBackend = (mIsOpenglBackend + 1) % 2;
			WriteAppSetting("Rendering Backend", "IsOpengl", newBackend);
			MessageBoxA(NULL, "rendering-backend will change after program restart", "switch rendering-backend", MB_OK);
		}
		CoReturn;
	};
	guiMng->AddCommand(cmd);
}

void GuiDebugWindow::AddRenderingPathSWCmd()
{
	auto sceneMng = mContext->SceneMng();
	auto guiMng = mContext->GuiMng();
	auto camera = sceneMng->GetDefCamera();

	auto canvas = sceneMng->CreateGuiCanvasNode();
	auto cmd = [=]()->CoTask<void>
	{
		//const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		//ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);

		//ImGui::Begin("rendering path");
		ImGui::Text("current rendering-path: %s", (mRenderingPath == kRenderPathForward ? "forward" : "deferred"));
		ImGui::SameLine();
		if (ImGui::Button("switch path")) {
			mRenderingPath = (mRenderingPath + 1) % 2;
			camera->SetRenderingPath((RenderingPath)mRenderingPath);
		}
		//ImGui::End();
		CoReturn;
	};
	guiMng->AddCommand(cmd);
}
void GuiDebugWindow::AddIBLPunctualSWCmd()
{
	auto sceneMng = mContext->SceneMng();
	auto guiMng = mContext->GuiMng();

	auto canvas = sceneMng->CreateGuiCanvasNode();
	auto cmd = [=]()->CoTask<void>
	{
		if (mRenderingPath == kRenderPathForward) {
			//ImGui::Begin("ibl && punctual");
		#define MACRO_CHECK_BOX(LABEL, VAR) if (ImGui::Checkbox(LABEL, &_##VAR)) CoAwait UpdateMtlsKeywords(#VAR, _##VAR);
			MACRO_CHECK_BOX("enable IBL", USE_IBL);
			MACRO_CHECK_BOX("enable Punctual", USE_PUNCTUAL);
			//ImGui::End();
		}
		CoReturn;
	};
	guiMng->AddCommand(cmd);
}
void GuiDebugWindow::AddDebugChannelCmd()
{
	auto sceneMng = mContext->SceneMng();
	auto guiMng = mContext->GuiMng();

	auto canvas = sceneMng->CreateGuiCanvasNode();
	auto cmd = [=]()->CoTask<void>
	{
		//ImGui::Begin("debug channel");
		if (mRenderingPath == kRenderPathForward) {
			const char* items[] = {
				"none", "shadow", "uv0", "uv1", "normal texture", "geometry normal",
				"geometry tangent", "geometry bitangent", "shading normal", "alpha", "ao",
				"emissive", "brdf diffuse", "brdf specular", "ibl diffuse", "ibl specular",
				"lut", "metallic roughness", "basecolor", "metallic", "perceptual roughness",
				"transmission", "sheen", "clear coat",
			};

			int item_macros[] = {
				DEBUG_CHANNEL_NONE, DEBUG_CHANNEL_SHADOW, DEBUG_CHANNEL_UV_0, DEBUG_CHANNEL_UV_1, DEBUG_CHANNEL_NORMAL_TEXTURE, DEBUG_CHANNEL_GEOMETRY_NORMAL,
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
				"none", "gbuffer-pos", "gbuffer-normal", "gbuffer-albedo", "shadow"
			};

			int item_macros[] = {
				DEBUG_CHANNEL_NONE, DEBUG_CHANNEL_GBUFFER_POS, DEBUG_CHANNEL_GBUFFER_NORMAL, DEBUG_CHANNEL_GBUFFER_ALBEDO, DEBUG_CHANNEL_SHADOW
			};

			if (ImGui::ListBox("debug channel", &_DEBUG_CHANNEL, items, IM_ARRAYSIZE(items), 8)) {
				int debug_channel = item_macros[_DEBUG_CHANNEL];
				CoAwait UpdateDeferredMtlKeywords("DEBUG_CHANNEL", debug_channel);
			}
		}

		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//ImGui::End();
		CoReturn;
	};
	guiMng->AddCommand(cmd);
}

void GuiDebugWindow::AddSSAOCmd()
{
	AddPostProcessCmd();

	auto sceneMng = mContext->SceneMng();
	auto guiMng = mContext->GuiMng();

	auto canvas = sceneMng->CreateGuiCanvasNode();
	auto cmd = [=]()->CoTask<void>
	{
		const char* items[] = {
			"none", "debug_ssao", "debug_ssao_blur"
		};

		int item_macros[] = {
			0, 1, 2
		};

		if (ImGui::ListBox("debug aaso", &_DEBUG_SSAO_CHANNEL, items, IM_ARRAYSIZE(items), 8)) {
			int debug_channel = item_macros[_DEBUG_SSAO_CHANNEL];
			CoAwait UpdateEffectMtlsKeywords("DEBUG_SSAO_CHANNEL", debug_channel);
		}
		CoReturn;
	};
	guiMng->AddCommand(cmd);
}

void GuiDebugWindow::AddPostProcessCmd()
{
	auto sceneMng = mContext->SceneMng();
	auto guiMng = mContext->GuiMng();

	auto canvas = sceneMng->CreateGuiCanvasNode();
	auto cmd = [=]()->CoTask<void>
	{
		ImGui::Text("post effect enabled: %s", (mEnablePostProcEffects ? "true" : "false"));
		ImGui::SameLine();
		if (ImGui::Button("switch ")) {
			mEnablePostProcEffects = (mEnablePostProcEffects + 1) % 2;
			
			for (auto& eff : mEffects) {
				eff->SetEnabled(mEnablePostProcEffects);
			}
		}
		CoReturn;
	};
	guiMng->AddCommand(cmd);
}
