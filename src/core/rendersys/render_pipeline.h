#pragma once
#include "core/base/std.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/material_cb.h"

namespace mir {

class RenderPipeline
{
	RenderSystem& mRenderSys;
	const int mScreenWidth, mScreenHeight;

	bool mCastShdowFlag = false;
	std::vector<IRenderTexturePtr> mRenderTargetStk;

	IRenderTexturePtr mShadowCasterOutput;
public:
	SceneManagerPtr mSceneManager;
public:
	RenderPipeline(RenderSystem& renderSys, int width, int height);
	bool BeginFrame();
	void EndFrame();
	void Render(const RenderOperationQueue& opQueue, SceneManager& scene);
	void Draw(IRenderable& renderable);
private:
	void _RenderSkyBox();
	void _DoPostProcess();
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();
	void BindPass(const PassPtr& pass, const cbGlobalParam& globalParam);
	void RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op, const cbGlobalParam& globalParam);
	void RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void MakeAutoParam(cbGlobalParam& param, bool castShadow, cbDirectLight* light, LightType lightType);
	void RenderLight(cbDirectLight* light, LightType lightType, const RenderOperationQueue& opQueue, const std::string& lightMode);
public:
	void RenderOpQueue(const RenderOperationQueue& opQueue, const std::string& lightMode);
};

}