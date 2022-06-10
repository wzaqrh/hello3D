#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/cppcoro.h"
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/base/launch.h"

namespace mir {

struct cbPerFrame;
struct cbPerLight;
class MIR_CORE_API RenderPipeline : boost::noncopyable
{
	friend class CameraRender;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, const Configure& cfg);
	~RenderPipeline();
	void Dispose();
	CoTask<bool> Initialize(Launch lchMode, ResourceManager& resMng);
	bool BeginFrame();
	void EndFrame();
	void Render(const RenderableCollection& rends, const std::vector<scene::CameraPtr>& cameras, const std::vector<scene::LightPtr>& lights);

	void GetDefferedMaterial(res::MaterialInstance& mtl) const;
private:
	void RenderCameraForward(const RenderableCollection& rends, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights);
	void RenderCameraDeffered(const RenderableCollection& rends, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights);
private:
	const Configure& mCfg;
	RenderSystem& mRenderSys;
	RenderStatesBlockPtr mStatesBlockPtr;
	RenderStatesBlock& mStatesBlock;
	FrameBufferBankPtr mFbsBank;
	IFrameBufferPtr mShadowMap, mGBuffer;
	rend::SpritePtr mGBufferSprite;
};

}