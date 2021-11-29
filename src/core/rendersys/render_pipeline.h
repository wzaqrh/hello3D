#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/stl.h"
#include "core/base/math.h"

namespace mir {

struct _declspec(align(16)) cbGlobalParam
{
	cbGlobalParam() {
		World = Eigen::Matrix4f::Identity();
		View = Eigen::Matrix4f::Identity();
		Projection = Eigen::Matrix4f::Identity();
		glstate_lightmodel_ambient = Eigen::Vector4f(0.01, 0.01, 0.01, 0.01);
	}
public:
	Eigen::Matrix4f World;
	Eigen::Matrix4f View;
	Eigen::Matrix4f Projection;

	Eigen::Matrix4f WorldInv;
	Eigen::Matrix4f ViewInv;
	Eigen::Matrix4f ProjectionInv;

	Eigen::Vector4f glstate_lightmodel_ambient;
};

struct cbPerLight;
class MIR_CORE_API RenderPipeline
{
	RenderSystem& mRenderSys;
	const Eigen::Vector2i mScreenSize;

	std::vector<IFrameBufferPtr> mFrameBufferStack;
	IFrameBufferPtr mShadowCasterOutput;
public:
	RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, const Eigen::Vector2i& size);
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
		const cbPerLight& lightParam, cbGlobalParam& globalParam);
	void RenderOpQueue(const RenderOperationQueue& opQueue, const Camera& camera, 
		const std::vector<ILightPtr>& lights, const std::string& lightMode);
	void RenderCamera(const RenderOperationQueue& opQueue, const Camera& camera,
		const std::vector<ILightPtr>& lights);
};

}