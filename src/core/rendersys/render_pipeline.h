#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/cppcoro.h"
#include "core/base/stl.h"
#include "core/base/math.h"

namespace mir {

struct cbPerFrame;
struct cbPerLight;
class MIR_CORE_API RenderPipeline : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng);
	bool BeginFrame();
	void EndFrame();
	void Render(const RenderOperationQueue& opQueue, const std::vector<scene::CameraPtr>& cameras, const std::vector<scene::LightPtr>& lights);
private:
	void BindPass(const res::PassPtr& pass);
	void RenderPass(const res::PassPtr& pass, const TextureVector& textures, int iterCnt, const RenderOperation& op);
	void RenderOp(const RenderOperation& op, const std::string& lightMode);
	void RenderLight(const RenderOperationQueue& opQue, const std::string& lightMode, unsigned camMask, const cbPerLight* perLight, cbPerFrame& perFrame);
	void RenderCameraForward(const RenderOperationQueue& opQue, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights);
	void RenderCameraDeffered(const RenderOperationQueue& opQue, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights);
private:
	RenderSystem& mRenderSys;
	RenderStatesBlockPtr mStatesBlockPtr;
	RenderStatesBlock& mStatesBlock;
	IFrameBufferPtr mShadowMap, mGBuffer;
	rend::SpritePtr mGBufferSprite;
};

}