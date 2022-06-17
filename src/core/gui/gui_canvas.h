#pragma once
#include "core/mir_export.h"
#include "core/gui/predeclare.h"
#define IMGUI_API MIR_CORE_API
#include "imgui/imgui.h"
#include "core/renderable/renderable.h"

namespace mir {
namespace gui {

class GuiCanvas : public Renderable
{
public:
	GuiCanvas(Launch lchMode, ResourceManager& resMng);
	~GuiCanvas();
	CoTask<bool> Init() ThreadMaySwitch;

	CoTask<void> UpdateFrame(float dt) override;
	void GenRenderOperation(RenderOperationQueue& ops) override;
	Eigen::AlignedBox3f GetWorldAABB() const override;
	void GetMaterials(std::vector<res::MaterialInstance>& mtls) const override;
private:
	void DoNewFrame();
	void DoSetupRenderState(ImDrawData* draw_data);
private:
	Launch mLchMode;
	ResourceManager& mResMng;
	IVertexArrayPtr mVao;
	RenderOperation mRop;
	ITexturePtr mFontTex;
private:
	std::vector<RenderOperation> mOps;
	int mVertexBufferSize = 5000;
	int mIndexBufferSize = 10000;
};

}
}