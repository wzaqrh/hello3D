#include "imgui_impl_win32.h"
#include "core/base/debug.h"
#include "core/gui/gui_canvas.h"
#include "core/resource/material.h"
#include "core/resource/material_factory.h"
#include "core/resource/texture_factory.h"
#include "core/resource/resource_manager.h"

struct VERTEX_CONSTANT_BUFFER
{
	float mvp[4][4];
};

namespace mir {
namespace gui {

GuiCanvas::GuiCanvas(Launch lchMode, ResourceManager& resMng)
	: mLchMode(lchMode)
	, mResMng(resMng)
{}
CoTask<bool> GuiCanvas::Init() ThreadMaySwitch
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");

	// Setup backend capabilities flags
	io.BackendRendererUserData = (void*)this;
	io.BackendRendererName = "imgui_impl_dx11";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

	BOOST_ASSERT(!mRop.Material);
	MaterialLoadParam mlp = "ImGui";
	mRop.Material = CoAwait mResMng.CreateMaterialT(__LaunchAsync__, mlp);

	{
		unsigned char* pixels;
		int width, height;
		ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		mFontTex = CoAwait mResMng.CreateTextureByDataT(mLchMode, kFormatR8G8B8A8UNorm, Eigen::Vector4i(width, height, 1, 1), &Data::Make(pixels, width * 4));
		CoAwait mResMng.SwitchToLaunchService(__LaunchSync__);

		ImGui::GetIO().Fonts->SetTexID((ImTextureID)mFontTex.get());
	}
	CoReturn true;
}

GuiCanvas::~GuiCanvas()
{
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = NULL;
	io.BackendRendererUserData = NULL;
	//IM_DELETE(bd);
}

void GuiCanvas::DoNewFrame()
{
	BOOST_ASSERT(mRop.Material);
}
void GuiCanvas::DoSetupRenderState(ImDrawData* draw_data)
{
	//mRenderSys->SetViewPort(0, 0, draw_data->DisplaySize.x, draw_data->DisplaySize.y);
}
CoTask<void> GuiCanvas::UpdateFrame(float dt)
{
	DEBUG_LOG_CALLSTK("guiCanvas.UpdateFrame");
	mOps.clear();

	this->DoNewFrame();

	ImDrawData* draw_data = ImGui::GetDrawData();
	if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) CoReturn;

	if (mRop.VertexBuffers.empty() || this->mVertexBufferSize < draw_data->TotalVtxCount)
	{
		this->mVertexBufferSize = draw_data->TotalVtxCount + 5000;

		mRop.VertexBuffers.resize(1);
		mRop.VertexBuffers[0] = mResMng.CreateVertexBuffer(mLchMode, sizeof(ImDrawVert), 0, Data::Make(nullptr, this->mVertexBufferSize * sizeof(ImDrawVert)));
	}

	if (!mRop.IndexBuffer || this->mIndexBufferSize < draw_data->TotalIdxCount)
	{
		this->mIndexBufferSize = draw_data->TotalIdxCount + 10000;

		mRop.IndexBuffer = mResMng.CreateIndexBuffer(mLchMode, sizeof(ImDrawIdx) == 2 ? kFormatR16UInt : kFormatR32UInt, Data::Make(nullptr, this->mIndexBufferSize * sizeof(ImDrawIdx)));
	}

	{
		std::vector<ImDrawVert> vtxs(this->mVertexBufferSize);
		int position = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			assert(position + cmd_list->VtxBuffer.Size <= vtxs.size());
			memcpy(&vtxs[position], cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			position += cmd_list->VtxBuffer.Size;
		}
		mResMng.UpdateBuffer(mRop.VertexBuffers[0], Data::Make(&vtxs[0], position * sizeof(ImDrawVert)));
	}

	{
		std::vector<ImDrawIdx> idxs(this->mIndexBufferSize);
		int position = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			assert(position + cmd_list->IdxBuffer.Size <= idxs.size());
			memcpy(&idxs[position], cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			position += cmd_list->IdxBuffer.Size;
		}
		mResMng.UpdateBuffer(mRop.IndexBuffer, Data::Make(&idxs[0], position * sizeof(ImDrawIdx)));
	}

	{
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,         0.0f,           0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};
		mRop.Material.SetProperty("ProjectionMatrix", Data::Make(mvp));
	}

	// Setup desired DX state
	this->DoSetupRenderState(draw_data);

	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_idx_offset = 0;
	int global_vtx_offset = 0;
	ImVec2 clip_off = draw_data->DisplayPos;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
		#if 0
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					DoSetupRenderState(draw_data);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			#endif
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
				ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
					continue;

				// Bind texture, Draw
				BOOST_ASSERT((ITexture*)pcmd->GetTexID() == mFontTex.get());
				mRop.Material.GetTextures().AddOrSet(mFontTex, 0);
				mRop.Scissor = ScissorState::Make((LONG)clip_min.x, (LONG)clip_min.y, (LONG)clip_max.x, (LONG)clip_max.y);
				mRop.IndexCount = pcmd->ElemCount;
				mRop.IndexPos = pcmd->IdxOffset + global_idx_offset;
				mRop.IndexBase = pcmd->VtxOffset + global_vtx_offset;
				mOps.push_back(mRop);
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}
	CoReturn;
}

void GuiCanvas::GenRenderOperation(RenderOperationQueue& ops)
{
	for (auto& mRop : mOps) {
		ops.AddOP(mRop);
	}
}
Eigen::AlignedBox3f GuiCanvas::GetWorldAABB() const
{
	return Eigen::AlignedBox3f();
}

void GuiCanvas::GetMaterials(std::vector<res::MaterialInstance>& mtls) const
{
	mtls.push_back(mRop.Material);
}

}
}