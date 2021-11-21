#include "core/rendersys/render_pipeline.h"
#include "core/base/debug.h"
#include "core/rendersys/interface_type.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/post_process.h"
#include "core/renderable/skybox.h"

namespace mir {

RenderPipeline::RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, int width, int height)
	:mRenderSys(renderSys)
	,mScreenWidth(width)
	,mScreenHeight(height)
{
	mShadowCasterOutput = resMng.CreateRenderTexture(Launch::Sync, mScreenWidth, mScreenHeight, kFormatR32Float);
	//SET_DEBUG_NAME(mShadowCasterOutput->mDepthStencilView, "shadow_caster_output");
}

void RenderPipeline::_PushRenderTarget(IRenderTexturePtr rendTarget)
{
	mRenderTargetStk.push_back(rendTarget);
	mRenderSys.SetRenderTarget(rendTarget);
}
void RenderPipeline::_PopRenderTarget()
{
	if (!mRenderTargetStk.empty())
		mRenderTargetStk.pop_back();

	mRenderSys.SetRenderTarget(!mRenderTargetStk.empty() ? mRenderTargetStk.back() : nullptr);
}

void RenderPipeline::RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op, const cbGlobalParam& globalParam)
{
	if (iterCnt >= 0) _PushRenderTarget(pass->mIterTargets[iterCnt]);
	else if (pass->mRenderTarget) _PushRenderTarget(pass->mRenderTarget);

	if (iterCnt >= 0) {
		if (iterCnt + 1 < pass->mIterTargets.size())
			textures[0] = pass->mIterTargets[iterCnt + 1]->GetColorTexture();
	}
	else {
		if (!pass->mIterTargets.empty())
			textures[0] = pass->mIterTargets[0]->GetColorTexture();
	}

	{
		if (textures.Count() > 0)
			mRenderSys.SetTextures(E_TEXTURE_MAIN, &textures.Textures[0], textures.Textures.size());

		if (pass->OnBind)
			pass->OnBind(*pass, mRenderSys, textures);

		BindPass(pass, globalParam);

		if (op.mIndexBuffer) mRenderSys.DrawIndexedPrimitive(op, pass->mTopoLogy);
		else mRenderSys.DrawPrimitive(op, pass->mTopoLogy);

		if (pass->OnUnbind)
			pass->OnUnbind(*pass, mRenderSys, textures);
	}

	if (iterCnt >= 0) {
		_PopRenderTarget();
	}
	else {
		if (pass->mRenderTarget)
			_PopRenderTarget();
	}
}

void RenderPipeline::BindPass(const PassPtr& pass, const cbGlobalParam& globalParam)
{
	mRenderSys.SetProgram(pass->mProgram);

	if (pass->mConstantBuffers.size() > 0)
		mRenderSys.UpdateBuffer(pass->mConstantBuffers[0].Buffer, (void*)&globalParam, sizeof(globalParam));

	auto cbuffers = pass->GetConstBuffers();
	mRenderSys.SetConstBuffers(0, &cbuffers[0], cbuffers.size(), pass->mProgram);

	mRenderSys.SetVertexLayout(pass->mInputLayout);

	if (!pass->mSamplers.empty())
		mRenderSys.SetSamplers(0, &pass->mSamplers[0], pass->mSamplers.size());
}

void RenderPipeline::RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam)
{
	TechniquePtr tech = op.mMaterial->CurTech();
	std::vector<PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes) {
		//SetVertexLayout(pass->mInputLayout);
		mRenderSys.SetVertexBuffer(op.mVertexBuffer);
		mRenderSys.SetIndexBuffer(op.mIndexBuffer);

		TextureBySlot textures = op.mTextures;
		textures.Merge(pass->mTextures);

		for (int i = pass->mIterTargets.size() - 1; i >= 0; --i) {
			auto iter = op.mVertBufferByPass.find(std::make_pair(pass, i));
			if (iter != op.mVertBufferByPass.end()) mRenderSys.SetVertexBuffer(iter->second);
			else mRenderSys.SetVertexBuffer(op.mVertexBuffer);
			
			ITexturePtr first = !textures.Empty() ? textures[0] : nullptr;
			RenderPass(pass, textures, i, op, globalParam);
			textures[0] = first;
		}
		auto iter = op.mVertBufferByPass.find(std::make_pair(pass, -1));
		if (iter != op.mVertBufferByPass.end()) mRenderSys.SetVertexBuffer(iter->second);
		else mRenderSys.SetVertexBuffer(op.mVertexBuffer);
		
		RenderPass(pass, textures, -1, op, globalParam);
	}
}

void RenderPipeline::RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, cbGlobalParam& globalParam)
{
	for (int i = 0; i < opQueue.Count(); ++i) {
		if (opQueue[i].mMaterial->IsLoaded()) {
			globalParam.World = opQueue[i].mWorldTransform;
			globalParam.WorldInv = globalParam.World.inverse();
			RenderOp(opQueue[i], lightMode, globalParam);
		}
	}
}

cbGlobalParam MakeAutoParam(const Camera& camera, bool castShadow, 
	cbDirectLight* light, LightType lightType)
{
	cbGlobalParam globalParam;
	memset(&globalParam, 0, sizeof(globalParam));

	if (castShadow) {
		light->CalculateLightingViewProjection(camera, globalParam.View, globalParam.Projection);
	}
	else {
		globalParam.View = camera.GetView();
		globalParam.Projection = camera.GetProjection();
		light->CalculateLightingViewProjection(camera, globalParam.LightView, globalParam.LightProjection);
	}
	globalParam.HasDepthMap = castShadow ? TRUE : FALSE;

	globalParam.WorldInv = globalParam.World.inverse();
	globalParam.ViewInv = globalParam.View.inverse();
	globalParam.ProjectionInv = globalParam.Projection.inverse();

	globalParam.LightType = lightType + 1;
	switch (lightType) {
	case kLightDirectional:
		static_cast<cbDirectLight&>(globalParam.Light) = *light;
		break;
	case kLightPoint:
		static_cast<cbPointLight&>(globalParam.Light) = *(cbPointLight*)light;
		break;
	case kLightSpot:
		globalParam.Light = *(cbSpotLight*)light;
		break;
	default:
		break;
	}
	return globalParam;
}

void RenderPipeline::RenderOpQueue(const RenderOperationQueue& opQueue, const Camera& camera, 
	const std::vector<std::pair<cbDirectLight*, LightType>>& lightsOrder, const std::string& lightMode)
{
	if (opQueue.IsEmpty()) return;

	DepthState orgState = mRenderSys.GetDepthState();
	BlendState orgBlend = mRenderSys.GetBlendFunc();

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PushRenderTarget(mShadowCasterOutput);
		mRenderSys.ClearColorDepthStencil(Eigen::Vector4f(1, 1, 1, 1), 1.0, 0);
		mRenderSys.SetDepthState(DepthState::MakeFor2D(false));
		mRenderSys.SetBlendFunc(BlendState::MakeDisable());
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, mShadowCasterOutput->GetColorTexture());

		auto& skyBox = camera.SkyBox();
		if (skyBox && skyBox->GetTexture())
			mRenderSys.SetTexture(E_TEXTURE_ENV, skyBox->GetTexture());
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		if (camera.mPostProcessInput) 
			mRenderSys.SetTexture(E_TEXTURE_MAIN, camera.mPostProcessInput->GetColorTexture());
	}

	if (!lightsOrder.empty()) {
		BlendState orgBlend = mRenderSys.GetBlendFunc();
		mRenderSys.SetBlendFunc(BlendState::MakeAlphaNonPremultiplied());
		
		cbGlobalParam globalParam = MakeAutoParam(camera, lightMode == E_PASS_SHADOWCASTER, 
			lightsOrder[0].first, lightsOrder[0].second);
		RenderLight(opQueue, lightMode, globalParam);

		for (int i = 1; i < lightsOrder.size(); ++i) {
			mRenderSys.SetBlendFunc(BlendState::MakeAdditive());
			auto lightModeEx = (lightMode == E_PASS_FORWARDBASE) ? E_PASS_FORWARDADD : lightMode;
			globalParam = MakeAutoParam(camera, lightModeEx == E_PASS_SHADOWCASTER, lightsOrder[i].first, lightsOrder[i].second);
			RenderLight(opQueue, lightModeEx, globalParam);
		}
		mRenderSys.SetBlendFunc(orgBlend);
	}

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PopRenderTarget();
		mRenderSys.SetDepthState(orgState);
		mRenderSys.SetBlendFunc(orgBlend);

		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
		mRenderSys.SetTexture(E_TEXTURE_ENV, nullptr);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
	}
}

void RenderPipeline::RenderCamera(const RenderOperationQueue& opQueue, const Camera& camera, 
	const std::vector<std::pair<cbDirectLight*, LightType>>& lights)
{
	RenderOpQueue(opQueue, camera, lights, E_PASS_SHADOWCASTER);
	RenderOpQueue(opQueue, camera, lights, E_PASS_FORWARDBASE);
	RenderOpQueue(opQueue, camera, lights, E_PASS_POSTPROCESS);
}

void RenderPipeline::Render(const RenderOperationQueue& opQueue, SceneManager& scene)
{
	for (auto& camera : scene.mCameras) 
	{
		//setup framebuffer as camera's post_process_input 
		if (!camera->PostProcessEffects().empty() && camera->mPostProcessInput) {
			mRenderSys.SetRenderTarget(camera->mPostProcessInput);
			mRenderSys.ClearColorDepthStencil(Eigen::Vector4f(0, 0, 0, 0), 1.0, 0);
		}

		//camera's skybox
		if (camera->SkyBox()) {
			RenderOperationQueue opQue;
			camera->SkyBox()->GenRenderOperation(opQue);
			RenderOpQueue(opQue, *camera, scene.mLightsByOrder, E_PASS_FORWARDBASE);
		}

		RenderCamera(opQueue, *camera, scene.mLightsByOrder);

		//camera's postprocess
		{
			DepthState orgState = mRenderSys.GetDepthState();
			mRenderSys.SetDepthState(DepthState::MakeFor2D(false));

			RenderOperationQueue opQue;
			auto& postProcessEffects = camera->PostProcessEffects();
			for (size_t i = 0; i < postProcessEffects.size(); ++i)
				postProcessEffects[i]->GenRenderOperation(opQue);
			RenderOpQueue(opQue, *camera, scene.mLightsByOrder, E_PASS_POSTPROCESS);

			mRenderSys.SetDepthState(orgState);
		}
	}
}

void RenderPipeline::_RenderSkyBox()
{
#if REFACTOR
	if (mSceneManager->GetDefCamera()->SkyBox()) {
		RenderOperationQueue opQue;
		mSceneManager->GetDefCamera()->SkyBox()->GenRenderOperation(opQue);
		RenderOpQueue(opQue, E_PASS_FORWARDBASE);
	}
#endif
}
void RenderPipeline::_DoPostProcess()
{
#if REFACTOR
	DepthState orgState = mRenderSys.GetDepthState();
	mRenderSys.SetDepthState(DepthState(false));

	RenderOperationQueue opQue;
	auto& postProcessEffects = mSceneManager->GetDefCamera()->PostProcessEffects();
	for (size_t i = 0; i < postProcessEffects.size(); ++i)
		postProcessEffects[i]->GenRenderOperation(opQue);
	RenderOpQueue(opQue, E_PASS_POSTPROCESS);

	mRenderSys.SetDepthState(orgState);
#endif
}

bool RenderPipeline::BeginFrame()
{
	if (!mRenderSys.BeginScene()) return false;

#if REFACTOR
	if (!mSceneManager->GetDefCamera()->PostProcessEffects().empty() && mSceneManager->GetDefCamera()->mPostProcessInput) {
		mRenderSys.SetRenderTarget(mSceneManager->GetDefCamera()->mPostProcessInput);
		mRenderSys.ClearColorDepthStencil(Eigen::Vector4f(0, 0, 0, 0), 1.0, 0);
	}
	_RenderSkyBox();
#endif
	return true;
}
void RenderPipeline::EndFrame()
{
#if REFACTOR
	if (!mSceneManager->GetDefCamera()->PostProcessEffects().empty()) {
		mRenderSys.SetRenderTarget(nullptr);
	}
	_DoPostProcess();
#endif
	mRenderSys.EndScene();
}

void RenderPipeline::Draw(IRenderable& renderable, SceneManager& scene)
{
	if (BeginFrame()) {
		RenderOperationQueue opQue;
		renderable.GenRenderOperation(opQue);
		Render(opQue, scene);
		EndFrame();
	}
}

}