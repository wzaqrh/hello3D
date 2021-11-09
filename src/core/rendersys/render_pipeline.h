#pragma once
#include "core/base/std.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/material_cb.h"

namespace mir {

struct RenderPipeline
{
	size_t mDrawCount = 0, mDrawLimit = INT_MAX;
	bool mCastShdowFlag = false;
	std::vector<IRenderTexturePtr> mRenderTargetStk;
	IRenderTexturePtr mShadowPassRT, mPostProcessRT;
public:
	int mScreenWidth, mScreenHeight;
	SceneManagerPtr mSceneManager;
	IRenderSystemPtr mRenderSys;
public:
	RenderPipeline(IRenderSystemPtr renderSys, int width, int height);
	void Draw(IRenderable& renderable);
	bool BeginFrame();
	void EndFrame();
private:
	void _RenderSkyBox();
	void _DoPostProcess();
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();
	void RenderOpQueue(const RenderOperationQueue& opQueue, const std::string& lightMode);
	void MakeAutoParam(cbGlobalParam& param, CameraBase* pLightCam, bool castShadow, cbDirectLight* light, LightType lightType);
	void RenderLight(cbDirectLight* light, LightType lightType, const RenderOperationQueue& opQueue, const std::string& lightMode);
	void RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op, const cbGlobalParam& globalParam);
	void BindPass(const PassPtr& pass, const cbGlobalParam& globalParam);
};

}