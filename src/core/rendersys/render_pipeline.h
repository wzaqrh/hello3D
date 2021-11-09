#pragma once
#include "core/base/std.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/material_cb.h"

namespace mir {

class RenderPipeline
{
	IRenderSystemPtr mRenderSys;
	const int mScreenWidth, mScreenHeight;

	bool mCastShdowFlag = false;
	std::vector<IRenderTexturePtr> mRenderTargetStk;
public:
	IRenderTexturePtr mShadowPassRT, mPostProcessRT;
	SceneManagerPtr mSceneManager;
public:
	RenderPipeline(IRenderSystemPtr renderSys, int width, int height);
	bool BeginFrame();
	void EndFrame();
	void Draw(IRenderable& renderable);
	void RenderOpQueue(const RenderOperationQueue& opQueue, const std::string& lightMode);
private:
	void _RenderSkyBox();
	void _DoPostProcess();
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();
	void MakeAutoParam(cbGlobalParam& param, CameraBase* pLightCam, bool castShadow, cbDirectLight* light, LightType lightType);
	void RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderLight(cbDirectLight* light, LightType lightType, const RenderOperationQueue& opQueue, const std::string& lightMode);
	void RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op, const cbGlobalParam& globalParam);
	void BindPass(const PassPtr& pass, const cbGlobalParam& globalParam);
};

}