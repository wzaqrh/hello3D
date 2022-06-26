#define _CRT_SECURE_NO_WARNINGS
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdint.h>
#include <boost/assert.hpp>
#include "core/mir.h"
#include "core/base/input.h"
#include "core/base/macros.h"
#include "core/rendersys/ogl/render_system_ogl.h"
#include "core/rendersys/render_pipeline.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_factory.h"
#include "core/resource/texture_factory.h"
#include "core/renderable/renderable.h"
#include "core/scene/camera.h"
#include "core/scene/scene_manager.h"

using namespace mir;

#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#define IMGUI_IMPL_HAS_POLYGON_MODE
#define IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
#define IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
#define IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
#define IMGUI_IMPL_OPENGL_MAY_HAVE_EXTENSIONS

// OpenGL Data
struct ImGui_ImplOpenGL3_Data : public Renderable
{
	ITexturePtr FontTexture;
	size_t  VertexBufferSize = 0;
	size_t IndexBufferSize = 0;
	Mir* Ctx = nullptr;
	RenderOperation Rop;
	std::vector<RenderOperation> Ops;
public:
	Eigen::AlignedBox3f GetWorldAABB() const override { return Eigen::AlignedBox3f(); }
	void GenRenderOperation(RenderOperationQueue& ops) override {
		for (auto& op : Ops)
			ops.AddOP(op);
	}
};
using ImGui_ImplOpenGL3_DataPtr = std::shared_ptr<ImGui_ImplOpenGL3_Data>;
ImGui_ImplOpenGL3_DataPtr bd = nullptr;

constexpr GLuint AttribLocationProjMtx = 3;
constexpr GLuint AttribLocationTex = 0;       // Uniforms location
constexpr GLuint AttribLocationVtxPos = 0;
constexpr GLuint AttribLocationVtxUV = 1;
constexpr GLuint AttribLocationVtxColor = 2;

bool ImGui_ImplOpenGL3_Init(void* hwnd, int w, int h)
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");

	// Setup backend capabilities flags
	bd = std::make_shared<ImGui_ImplOpenGL3_Data>();
	io.BackendRendererUserData = (void*)bd.get();
	io.BackendRendererName = "imgui_impl_opengl3";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	bd->Ctx = new Mir(LaunchSync);
	bd->Ctx->ExecuteTaskSync(bd->Ctx->Initialize((HWND)hwnd, R"(C:/mir/bin/work/)"));

	bd->Ctx->SceneMng()->CreateCameraNode(kCameraPerspective);
	return true;
}

void ImGui_ImplOpenGL3_Shutdown()
{
	IM_ASSERT(bd != NULL && "No renderer backend to shutdown, or already shutdown?");
	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	io.BackendRendererName = NULL;
	io.BackendRendererUserData = NULL;
	bd = nullptr;
}

void ImGui_ImplOpenGL3_NewFrame()
{
	IM_ASSERT(bd != NULL && "Did you call ImGui_ImplOpenGL3_Init()?");

	if (!bd->Rop.Material)
		ImGui_ImplOpenGL3_CreateDeviceObjects();
}

struct Matrix4x4f {
	float m[4][4];
};

void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data, ImVec4 clrColor)
{
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	{
		//bd->Ctx->RenderSys()->SetViewPort(0, 0, fb_width, fb_height);

		//bd->Ctx->RenderSys()->SetBlendState(BlendState::Make(kBlendSrcAlpha, kBlendInvSrcAlpha));
		//bd->Ctx->RenderSys()->SetDepthState(DepthState::MakeDisable());
		//bd->Ctx->RenderSys()->SetCullMode(kCullNone);
		//bd->Ctx->RenderSys()->SetFillMode(kFillSolid);

		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		const Matrix4x4f ortho_projection =
		{ {
			{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
		} };
		Eigen::Matrix4f m = *(Eigen::Matrix4f*)&ortho_projection;
		bd->Rop.Material.SetProperty("ProjectionMatrix", m);
	}

	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	bd->Ops.clear();
	auto resMng = bd->Ctx->ResourceMng();

	auto vaoHandle = NULLABLE(bd->Rop.IndexBuffer, GetVAO());
	if (vaoHandle == nullptr)
	{
		vaoHandle = resMng->CreateVertexArray(__LaunchSync__);
	}
	if (bd->Rop.VertexBuffers.empty() || bd->VertexBufferSize < draw_data->TotalVtxCount)
	{
		bd->VertexBufferSize = draw_data->TotalVtxCount + 5000;

		bd->Rop.VertexBuffers.resize(1);
		bd->Rop.VertexBuffers[0] = resMng->CreateVertexBuffer(__LaunchSync__, vaoHandle, sizeof(ImDrawVert), 0, Data::MakeSize(bd->VertexBufferSize * sizeof(ImDrawVert)));
	}
	if (bd->Rop.IndexBuffer == NULL || bd->IndexBufferSize < draw_data->TotalIdxCount)
	{
		bd->IndexBufferSize = draw_data->TotalIdxCount + 10000;

		bd->Rop.IndexBuffer = resMng->CreateIndexBuffer(__LaunchSync__, vaoHandle, sizeof(ImDrawIdx) == 4 ? kFormatR32UInt : kFormatR16UInt, Data::MakeSize(bd->IndexBufferSize * sizeof(ImDrawIdx)));
	}

	if (draw_data->CmdListsCount)
	{
		std::vector<ImDrawVert> vtxs(bd->VertexBufferSize);
		int position = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			BOOST_ASSERT(position + cmd_list->VtxBuffer.Size <= vtxs.size());
			memcpy(&vtxs[position], cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			position += cmd_list->VtxBuffer.Size;
		}
		resMng->UpdateBuffer(bd->Rop.VertexBuffers[0], Data::Make(&vtxs[0], position * sizeof(ImDrawVert)));
	}

	if (draw_data->CmdListsCount)
	{
		std::vector<ImDrawIdx> idxs(bd->IndexBufferSize);
		int position = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			BOOST_ASSERT(position + cmd_list->IdxBuffer.Size <= idxs.size());
			memcpy(&idxs[position], cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			position += cmd_list->IdxBuffer.Size;
		}
		resMng->UpdateBuffer(bd->Rop.IndexBuffer, Data::Make(&idxs[0], position * sizeof(ImDrawIdx)));
	}

	int global_idx_offset = 0;
	int global_vtx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
				ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
					continue;

				bd->Ops.push_back(bd->Rop);
				RenderOperation& op = bd->Ops.back();
				op.Scissor = ScissorState::Make(clip_min.x, clip_min.y, clip_max.x, clip_max.y);
				op.IndexCount = pcmd->ElemCount;
				op.IndexPos = pcmd->IdxOffset + global_idx_offset;
				op.IndexBase = pcmd->VtxOffset + global_vtx_offset;
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}

	auto scene = bd->Ctx->SceneMng();
	scene->GetDefCamera()->SetBackgroundColor(Eigen::Vector4f(clrColor.x * clrColor.w, clrColor.y * clrColor.w, clrColor.z * clrColor.w, clrColor.w));

	auto pipeline = bd->Ctx->RenderPipe();
	if (pipeline->BeginFrame()) {
		RenderableCollection collect;
		collect.AddRenderable(bd);

		pipeline->Render(collect, scene->GetCameras(), scene->GetLights());
		pipeline->EndFrame();
	}
}

bool ImGui_ImplOpenGL3_CreateFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();

	// Build texture atlas
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
	constexpr int mipCount = 1, faceCount = 1;

	bd->FontTexture = bd->Ctx->ResourceMng()->CreateTextureByDataS(LaunchSync, kFormatR8G8B8A8UNorm, Eigen::Vector4i(width, height, mipCount, faceCount), &Data2::Make(pixels, height * width * 4, width * 4));
	io.Fonts->SetTexID((ImTextureID)bd->FontTexture.get());

	bd->Rop.Material.GetTextures().AddOrSet(bd->FontTexture, 0);
	return true;
}

void ImGui_ImplOpenGL3_DestroyFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();
}

bool ImGui_ImplOpenGL3_CreateDeviceObjects()
{
	MaterialLoadParam mlp = "ImGui";
	bd->Rop.Material = bd->Ctx->ResourceMng()->CreateMaterialS(LaunchSync, mlp);

	ImGui_ImplOpenGL3_CreateFontsTexture();
	return true;
}

void ImGui_ImplOpenGL3_DestroyDeviceObjects()
{
	ImGui_ImplOpenGL3_DestroyFontsTexture();
}
