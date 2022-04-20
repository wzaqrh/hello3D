#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/cppcoro.h"
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/base/base_type.h"

namespace mir {

class TempFrameBufferManager {
public:
	TempFrameBufferManager(ResourceManager& resMng, Eigen::Vector2i fbSize, std::vector<ResourceFormat> fmts) :mResMng(resMng), mFbSize(fbSize) ,mFbFormats(fmts) {}
	void ReturnAll();
	IFrameBufferPtr Borrow();
private:
	ResourceManager& mResMng;
	Eigen::Vector2i mFbSize;
	std::vector<ResourceFormat> mFbFormats;
	std::vector<IFrameBufferPtr> mFbs;
	size_t mBorrowCount = 0;
};

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
	TempFrameBufferManagerPtr mTempFbMng;
	IFrameBufferPtr mShadowMap, mGBuffer;
	rend::SpritePtr mGBufferSprite;
};

}