#pragma once
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/stl.h"
#include "core/resource/material_cb.h"

namespace mir {

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

	Eigen::Matrix4f LightView;
	Eigen::Matrix4f LightProjection;
	cbSpotLight Light;

	unsigned int LightType;//directional=1,point=2,spot=3
	unsigned int HasDepthMap;
};

class MIR_CORE_API RenderPipeline
{
	RenderSystem& mRenderSys;
	const Eigen::Vector2i mScreenSize;

	std::vector<IRenderTexturePtr> mRenderTargetStk;
	IRenderTexturePtr mShadowCasterOutput;
public:
	RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, const Eigen::Vector2i& size);
	bool BeginFrame();
	void EndFrame();
	void Render(const RenderOperationQueue& opQueue, SceneManager& scene);
	void Draw(IRenderable& renderable, SceneManager& scene);
private:
	void _RenderSkyBox();
	void _DoPostProcess();
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();
	void BindPass(const PassPtr& pass, const cbGlobalParam& globalParam);
	void RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op, const cbGlobalParam& globalParam);
	void RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, cbGlobalParam& globalParam);
	void RenderOpQueue(const RenderOperationQueue& opQueue, const Camera& camera, 
		const std::vector<std::pair<cbDirectLight*, int>>& lights, const std::string& lightMode);
	void RenderCamera(const RenderOperationQueue& opQueue, const Camera& camera,
		const std::vector<std::pair<cbDirectLight*, int>>& lights);
};

}