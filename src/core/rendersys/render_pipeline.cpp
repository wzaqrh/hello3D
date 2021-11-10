#include "core/rendersys/render_pipeline.h"
#include "core/base/utility.h"
#include "core/rendersys/interface_type.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/post_process.h"
#include "core/renderable/skybox.h"

namespace mir {

RenderPipeline::RenderPipeline(IRenderSystemPtr renderSys, int width, int height)
	:mRenderSys(renderSys)
	,mScreenWidth(width)
	,mScreenHeight(height)
{
	mShadowPassRT = mRenderSys->CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R32_FLOAT);
	SET_DEBUG_NAME(mShadowPassRT->mDepthStencilView, "mShadowPassRT");

	mPostProcessRT = mRenderSys->CreateRenderTexture(mScreenWidth, mScreenHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
	SET_DEBUG_NAME(mPostProcessRT->mDepthStencilView, "mPostProcessRT");
}

void RenderPipeline::_PushRenderTarget(IRenderTexturePtr rendTarget)
{
	mRenderTargetStk.push_back(rendTarget);
	mRenderSys->SetRenderTarget(rendTarget);
}
void RenderPipeline::_PopRenderTarget()
{
	if (!mRenderTargetStk.empty())
		mRenderTargetStk.pop_back();

	mRenderSys->SetRenderTarget(!mRenderTargetStk.empty() ? mRenderTargetStk.back() : nullptr);
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
			mRenderSys->SetTextures(E_TEXTURE_MAIN, &textures.Textures[0], textures.Textures.size());

		if (pass->OnBind)
			pass->OnBind(*pass, *mRenderSys, textures);

		BindPass(pass, globalParam);

		if (op.mIndexBuffer) mRenderSys->DrawIndexedPrimitive(op, pass->mTopoLogy);
		else mRenderSys->DrawPrimitive(op, pass->mTopoLogy);

		if (pass->OnUnbind)
			pass->OnUnbind(*pass, *mRenderSys, textures);
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
	mRenderSys->SetProgram(pass->mProgram);

	if (pass->mConstantBuffers.size() > 0)
		mRenderSys->UpdateConstBuffer(pass->mConstantBuffers[0].Buffer, (void*)&globalParam, sizeof(globalParam));

	auto cbuffers = pass->GetConstBuffers();
	mRenderSys->SetConstBuffers(0, &cbuffers[0], cbuffers.size(), pass->mProgram);

	mRenderSys->SetVertexLayout(pass->mInputLayout);

	if (!pass->mSamplers.empty())
		mRenderSys->SetSamplers(0, &pass->mSamplers[0], pass->mSamplers.size());
}

void RenderPipeline::RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam)
{
	TechniquePtr tech = op.mMaterial->CurTech();
	std::vector<PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes) {
		//SetVertexLayout(pass->mInputLayout);
		mRenderSys->SetVertexBuffer(op.mVertexBuffer);
		mRenderSys->SetIndexBuffer(op.mIndexBuffer);

		TextureBySlot textures = op.mTextures;
		textures.Merge(pass->mTextures);

		for (int i = pass->mIterTargets.size() - 1; i >= 0; --i) {
			auto iter = op.mVertBufferByPass.find(std::make_pair(pass, i));
			if (iter != op.mVertBufferByPass.end()) mRenderSys->SetVertexBuffer(iter->second);
			else mRenderSys->SetVertexBuffer(op.mVertexBuffer);
			
			ITexturePtr first = !textures.Empty() ? textures[0] : nullptr;
			RenderPass(pass, textures, i, op, globalParam);
			textures[0] = first;
		}
		auto iter = op.mVertBufferByPass.find(std::make_pair(pass, -1));
		if (iter != op.mVertBufferByPass.end()) mRenderSys->SetVertexBuffer(iter->second);
		else mRenderSys->SetVertexBuffer(op.mVertexBuffer);
		
		RenderPass(pass, textures, -1, op, globalParam);
	}
}

void RenderPipeline::MakeAutoParam(cbGlobalParam& globalParam, bool castShadow, cbDirectLight* light, LightType lightType)
{
	memset(&globalParam, 0, sizeof(globalParam));

	if (castShadow) {
		light->CalculateLightingViewProjection(*mSceneManager->mDefCamera, globalParam.View, globalParam.Projection);
	}
	else {
		globalParam.View = mSceneManager->mDefCamera->GetView();
		globalParam.Projection = mSceneManager->mDefCamera->GetProjection();
		light->CalculateLightingViewProjection(*mSceneManager->mDefCamera, globalParam.LightView, globalParam.LightProjection);
	}
	globalParam.HasDepthMap = mCastShdowFlag ? TRUE : FALSE;

	globalParam.WorldInv = XM::Inverse(globalParam.World);
	globalParam.ViewInv = XM::Inverse(globalParam.View);
	globalParam.ProjectionInv = XM::Inverse(globalParam.Projection);

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
}
void RenderPipeline::RenderLight(cbDirectLight* light, LightType lightType, const RenderOperationQueue& opQueue, const std::string& lightMode)
{
	cbGlobalParam globalParam;
	MakeAutoParam(globalParam, lightMode == E_PASS_SHADOWCASTER, light, lightType);

	for (int i = 0; i < opQueue.Count(); ++i) {
		if (opQueue[i].mMaterial->IsLoaded()) {
			globalParam.World = opQueue[i].mWorldTransform;
			globalParam.WorldInv = XM::Inverse(globalParam.World);
			RenderOp(opQueue[i], lightMode, globalParam);
		}
	}
}

void RenderPipeline::RenderOpQueue(const RenderOperationQueue& opQueue, const std::string& lightMode)
{
	DepthState orgState = mRenderSys->GetDepthState();
	BlendFunc orgBlend = mRenderSys->GetBlendFunc();

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PushRenderTarget(mShadowPassRT);
		mRenderSys->ClearColorDepthStencil(XMFLOAT4(1, 1, 1, 1), 1.0, 0);
		mRenderSys->SetDepthState(DepthState(false));
		mRenderSys->SetBlendFunc(BlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_ZERO));
		mCastShdowFlag = true;
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		mRenderSys->SetTexture(E_TEXTURE_DEPTH_MAP, mShadowPassRT->GetColorTexture());

		auto& skyBox = mSceneManager->mSkyBox;
		if (skyBox && skyBox->mCubeSRV)
			mRenderSys->SetTexture(E_TEXTURE_ENV, skyBox->mCubeSRV);
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		mRenderSys->SetTexture(E_TEXTURE_MAIN, mPostProcessRT->GetColorTexture());
	}

	auto& lightsOrder = mSceneManager->mLightsByOrder;
	if (!lightsOrder.empty()) {
		BlendFunc orgBlend = mRenderSys->GetBlendFunc();
		mRenderSys->SetBlendFunc(BlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA));
		RenderLight(lightsOrder[0].first, lightsOrder[0].second, opQueue, lightMode);

		for (int i = 1; i < lightsOrder.size(); ++i) {
			auto order = lightsOrder[i];
			mRenderSys->SetBlendFunc(BlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE));
			auto __lightMode = (lightMode == E_PASS_FORWARDBASE) ? E_PASS_FORWARDADD : lightMode;
			RenderLight(order.first, order.second, opQueue, __lightMode);
		}
		mRenderSys->SetBlendFunc(orgBlend);
	}

	if (lightMode == E_PASS_SHADOWCASTER) {
		_PopRenderTarget();
		mRenderSys->SetDepthState(orgState);
		mRenderSys->SetBlendFunc(orgBlend);

		mRenderSys->SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
		mRenderSys->SetTexture(E_TEXTURE_ENV, nullptr);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		mRenderSys->SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
	}
}

void RenderPipeline::_RenderSkyBox()
{
	if (mSceneManager->mSkyBox) {
		RenderOperationQueue opQue;
		mSceneManager->mSkyBox->GenRenderOperation(opQue);
		RenderOpQueue(opQue, E_PASS_FORWARDBASE);
	}
}
void RenderPipeline::_DoPostProcess()
{
	DepthState orgState = mRenderSys->GetDepthState();
	mRenderSys->SetDepthState(DepthState(false));

	RenderOperationQueue opQue;
	for (size_t i = 0; i < mSceneManager->mPostProcs.size(); ++i)
		mSceneManager->mPostProcs[i]->GenRenderOperation(opQue);
	RenderOpQueue(opQue, E_PASS_POSTPROCESS);

	mRenderSys->SetDepthState(orgState);
}

bool RenderPipeline::BeginFrame()
{
	if (!mRenderSys->BeginScene()) return false;

	mCastShdowFlag = false;

	if (!mSceneManager->mPostProcs.empty()) {
		mRenderSys->SetRenderTarget(mPostProcessRT);
		mRenderSys->ClearColorDepthStencil(XMFLOAT4(0, 0, 0, 0), 1.0, 0);
	}
	_RenderSkyBox();
	return true;
}
void RenderPipeline::EndFrame()
{
	if (!mSceneManager->mPostProcs.empty()) {
		mRenderSys->SetRenderTarget(nullptr);
	}
	_DoPostProcess();

	mRenderSys->EndScene();
}

void RenderPipeline::Draw(IRenderable& renderable)
{
	if (BeginFrame()) {
		RenderOperationQueue opQue;
		renderable.GenRenderOperation(opQue);
		RenderOpQueue(opQue, E_PASS_FORWARDBASE);
		EndFrame();
	}
}

}