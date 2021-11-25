#include "core/rendersys/render_pipeline.h"
#include "core/rendersys/interface_type.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_factory.h"
#include "core/scene/scene_manager.h"
#include "core/scene/camera.h"
#include "core/renderable/post_process.h"
#include "core/renderable/skybox.h"
#include "core/base/debug.h"

namespace mir {

RenderPipeline::RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, const Eigen::Vector2i& size)
	:mRenderSys(renderSys)
	,mScreenSize(size)
{
	mShadowCasterOutput = resMng.CreateFrameBuffer(__LaunchSync__, mScreenSize, kFormatR32Float);
	//SET_DEBUG_NAME(mShadowCasterOutput->mDepthStencilView, "shadow_caster_output");
}

void RenderPipeline::_PushRenderTarget(IFrameBufferPtr rendTarget)
{
	mRenderTargetStk.push_back(rendTarget);
	mRenderSys.SetFrameBuffer(rendTarget);
}
void RenderPipeline::_PopRenderTarget()
{
	if (!mRenderTargetStk.empty())
		mRenderTargetStk.pop_back();

	mRenderSys.SetFrameBuffer(!mRenderTargetStk.empty() ? mRenderTargetStk.back() : nullptr);
}

void RenderPipeline::BindPass(const PassPtr& pass)
{
	mRenderSys.SetProgram(pass->mProgram);

	auto cbuffers = pass->GetConstBuffers();
	mRenderSys.SetConstBuffers(0, &cbuffers[0], cbuffers.size(), pass->mProgram);

	mRenderSys.SetVertexLayout(pass->mInputLayout);

	if (!pass->mSamplers.empty())
		mRenderSys.SetSamplers(0, &pass->mSamplers[0], pass->mSamplers.size());
}

void RenderPipeline::RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op)
{
	if (iterCnt >= 0) _PushRenderTarget(pass->mRTIterators[iterCnt]);
	else if (pass->mRenderTarget) _PushRenderTarget(pass->mRenderTarget);

	if (iterCnt >= 0) {
		if (iterCnt + 1 < pass->mRTIterators.size())
			textures[0] = pass->mRTIterators[iterCnt + 1]->GetColorTexture();
	}
	else {
		if (!pass->mRTIterators.empty())
			textures[0] = pass->mRTIterators[0]->GetColorTexture();
	}

	{
		if (textures.Count() > 0)
			mRenderSys.SetTextures(E_TEXTURE_MAIN, &textures.Textures[0], textures.Textures.size());

		if (op.OnBind) 
			op.OnBind(mRenderSys, *pass, op);

		for (auto& cbBytes : op.mCBDataByName)
			pass->UpdateConstBufferByName(mRenderSys, cbBytes.first, Data::Make(cbBytes.second));

		BindPass(pass);

		if (op.mIndexBuffer) mRenderSys.DrawIndexedPrimitive(op, pass->mTopoLogy);
		else mRenderSys.DrawPrimitive(op, pass->mTopoLogy);

		if (op.OnUnbind) 
			op.OnUnbind(mRenderSys, *pass, op);
	}

	if (iterCnt >= 0) {
		_PopRenderTarget();
	}
	else {
		if (pass->mRenderTarget)
			_PopRenderTarget();
	}
}

void RenderPipeline::RenderOp(const RenderOperation& op, const std::string& lightMode)
{
	TechniquePtr tech = op.mMaterial->CurTech();
	std::vector<PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes) {
		//SetVertexLayout(pass->mInputLayout);
		mRenderSys.SetVertexBuffer(op.mVertexBuffer);
		mRenderSys.SetIndexBuffer(op.mIndexBuffer);

		TextureBySlot textures = op.mTextures;
		textures.Merge(pass->mTextures);

		for (int i = pass->mRTIterators.size() - 1; i >= 0; --i) {
			auto iter = op.mVertBufferByPass.find(std::make_pair(pass, i));
			if (iter != op.mVertBufferByPass.end()) mRenderSys.SetVertexBuffer(iter->second);
			else mRenderSys.SetVertexBuffer(op.mVertexBuffer);
			
			ITexturePtr first = !textures.Empty() ? textures[0] : nullptr;
			RenderPass(pass, textures, i, op);
			textures[0] = first;
		}
		auto iter = op.mVertBufferByPass.find(std::make_pair(pass, -1));
		if (iter != op.mVertBufferByPass.end()) mRenderSys.SetVertexBuffer(iter->second);
		else mRenderSys.SetVertexBuffer(op.mVertexBuffer);
		
		RenderPass(pass, textures, -1, op);
	}
}

void RenderPipeline::RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, 
	const cbPerLight& lightParam, cbGlobalParam& globalParam)
{
	for (int i = 0; i < opQueue.Count(); ++i) {
		auto& op = opQueue[i];
		if (op.mMaterial->IsLoaded()) {
			globalParam.World = op.mWorldTransform;
			globalParam.WorldInv = globalParam.World.inverse();
			op.mMaterial->CurTech()->UpdateConstBufferByName(mRenderSys, 
				MAKE_CBNAME(cbGlobalParam), Data::Make(globalParam));
			
			op.mMaterial->CurTech()->UpdateConstBufferByName(mRenderSys,
				MAKE_CBNAME(cbPerLight), Data::Make(lightParam));

			RenderOp(op, lightMode);
		}
	}
}

std::tuple<cbGlobalParam, cbPerLight> MakeAutoParam(const Camera& camera, bool castShadow, const ILight& light)
{
	cbGlobalParam globalParam = {};
	cbPerLight lightParam = {};

	if (castShadow) {
		light.CalculateLightingViewProjection(camera, globalParam.View, globalParam.Projection);
	}
	else {
		globalParam.View = camera.GetView();
		globalParam.Projection = camera.GetProjection();
		light.CalculateLightingViewProjection(camera, lightParam.LightView, lightParam.LightProjection);
	}
	globalParam.WorldInv = globalParam.World.inverse();
	globalParam.ViewInv = globalParam.View.inverse();
	globalParam.ProjectionInv = globalParam.Projection.inverse();

	lightParam.HasDepthMap = castShadow ? TRUE : FALSE;
	lightParam.LightType = light.GetType() + 1;
	lightParam.Light = light.MakeCbLight();
	return std::tie(globalParam, lightParam);
}

void RenderPipeline::RenderOpQueue(const RenderOperationQueue& opQueue, const Camera& camera, 
	const std::vector<ILightPtr>& lightsOrder, const std::string& lightMode)
{
	if (opQueue.IsEmpty()) return;

	DepthState orgState = mRenderSys.GetDepthState();
	BlendState orgBlend = mRenderSys.GetBlendFunc();

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PushRenderTarget(mShadowCasterOutput);
		mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f(1, 1, 1, 1), 1.0, 0);
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
		
		cbGlobalParam globalParam;
		cbPerLight lightParam;
		std::tie(globalParam, lightParam) = MakeAutoParam(camera, lightMode == E_PASS_SHADOWCASTER, *lightsOrder[0]);
		RenderLight(opQueue, lightMode, lightParam, globalParam);

		for (int i = 1; i < lightsOrder.size(); ++i) {
			mRenderSys.SetBlendFunc(BlendState::MakeAdditive());
			auto lightModeEx = (lightMode == E_PASS_FORWARDBASE) ? E_PASS_FORWARDADD : lightMode;
			std::tie(globalParam, lightParam) = MakeAutoParam(camera, lightModeEx == E_PASS_SHADOWCASTER, *lightsOrder[i]);
			RenderLight(opQueue, lightModeEx, lightParam, globalParam);
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
	const std::vector<ILightPtr>& lights)
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
			mRenderSys.SetFrameBuffer(camera->mPostProcessInput);
			mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f(0, 0, 0, 0), 1.0, 0);
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

bool RenderPipeline::BeginFrame()
{
	return mRenderSys.BeginScene();
}
void RenderPipeline::EndFrame()
{
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