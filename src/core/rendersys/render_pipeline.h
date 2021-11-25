#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/stl.h"
#include "core/base/math.h"

namespace mir {

struct cbPerLight;

struct cbGlobalParam
{
	cbGlobalParam() {
		World = Eigen::Matrix4f::Identity();
		View = Eigen::Matrix4f::Identity();
		Projection = Eigen::Matrix4f::Identity();
	}
public:
	Eigen::Matrix4f World;
	Eigen::Matrix4f View;
	Eigen::Matrix4f Projection;

	Eigen::Matrix4f WorldInv;
	Eigen::Matrix4f ViewInv;
	Eigen::Matrix4f ProjectionInv;
};

class MIR_CORE_API RenderPipeline
{
	RenderSystem& mRenderSys;
	const Eigen::Vector2i mScreenSize;

	std::vector<IRenderTargetPtr> mRenderTargetStk;
	IRenderTargetPtr mShadowCasterOutput;
public:
	RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, const Eigen::Vector2i& size);
	bool BeginFrame();
	void EndFrame();
	void Render(const RenderOperationQueue& opQueue, SceneManager& scene);
	void Draw(IRenderable& renderable, SceneManager& scene);
private:
	void _RenderSkyBox();
	void _DoPostProcess();
	void _PushRenderTarget(IRenderTargetPtr rendTarget);
	void _PopRenderTarget();
	void BindPass(const PassPtr& pass);
	void RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op);
	void RenderOp(const RenderOperation& op, const std::string& lightMode);
	void RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, 
		const cbPerLight& lightParam, cbGlobalParam& globalParam);
	void RenderOpQueue(const RenderOperationQueue& opQueue, const Camera& camera, 
		const std::vector<ILightPtr>& lights, const std::string& lightMode);
	void RenderCamera(const RenderOperationQueue& opQueue, const Camera& camera,
		const std::vector<ILightPtr>& lights);
};

}