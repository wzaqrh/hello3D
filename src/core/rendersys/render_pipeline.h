#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/stl.h"
#include "core/base/math.h"

namespace mir {

struct cbPerFrame;
struct cbPerLight;
class MIR_CORE_API RenderPipeline
{
public:
	RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng);
	bool BeginFrame();
	void EndFrame();
	void Render(const RenderOperationQueue& opQueue, SceneManager& scene);
	void Draw(IRenderable& renderable, SceneManager& scene);
private:
	void _PushFrameBuffer(IFrameBufferPtr rendTarget);
	void _PopFrameBuffer();
	void BindPass(const PassPtr& pass);
	void RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op);
	void RenderOp(const RenderOperation& op, const std::string& lightMode);
	void RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, unsigned camMask, 
		const cbPerLight* lightParam, cbPerFrame& globalParam);
	void RenderCamera(const RenderOperationQueue& opQueue, const Camera& camera,
		const std::vector<ILightPtr>& lights);
private:
	RenderSystem& mRenderSys;

	std::vector<IFrameBufferPtr> mFrameBufferStack;
	IFrameBufferPtr mShadowMap;
};

}