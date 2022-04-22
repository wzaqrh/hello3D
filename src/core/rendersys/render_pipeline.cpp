#include <boost/format.hpp>
#include "core/rendersys/render_pipeline.h"
#include "core/rendersys/render_states_block.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_factory.h"
#include "core/renderable/sprite.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/scene/scene_manager.h"
#include "core/scene/light.h"
#include "core/scene/camera.h"
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/base/rendersys_debug.h"
#include "core/base/attribute_struct.h"
#include "test/unit_test/unit_test.h"

namespace mir {

void TempFrameBufferManager::ReturnAll()
{
	mBorrowCount = 0;
}

IFrameBufferPtr TempFrameBufferManager::Borrow()
{
	IFrameBufferPtr res = nullptr;
	if (mBorrowCount >= mFbs.size()) {
		for (size_t i = 0; i < 4; ++i) {
			auto tmpfb = mResMng.CreateFrameBuffer(__LaunchSync__, mFbSize, mFbFormats);
			DEBUG_SET_PRIV_DATA(tmpfb, (boost::format("TempFrameBufferManager.temp%d") % mFbs.size()).str().c_str());
			mFbs.push_back(tmpfb);
		}
	}
	res = mFbs[mBorrowCount];
	mBorrowCount++;
	return res;
}

enum PipeLineTextureSlot {
	kPipeTextureSceneImage = 7,
	kPipeTextureShadowMap = 8,
	kPipeTextureDiffuseEnv = 9,
	kPipeTextureSpecEnv = 10,
	kPipeTextureLUT = 11,
	kPipeTextureGBufferPos = 12,
	kPipeTextureGBufferNormal = 13,
	kPipeTextureGBufferAlbedo = 14,
	kPipeTextureGBufferEmissive = 15
};

RenderPipeline::RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng)
	: mRenderSys(renderSys)
	, mStatesBlockPtr(CreateInstance<RenderStatesBlock>(renderSys))
	, mStatesBlock(*mStatesBlockPtr)
{
	mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, renderSys.WinSize(), 
		MakeResFormats(kFormatUnknown, kFormatD24UNormS8UInt));
	DEBUG_SET_PRIV_DATA(mShadowMap, "render_pipeline.shadow_map");

	mGBuffer = resMng.CreateFrameBuffer(__LaunchSync__, renderSys.WinSize(), 
		MakeResFormats(kFormatR8G8B8A8UNorm, kFormatR8G8B8A8UNorm, kFormatR8G8B8A8UNorm, kFormatR8G8B8A8UNorm, kFormatD24UNormS8UInt));
	//mGBuffer->SetAttachZStencil(renderSys.GetBackFrameBuffer()->GetAttachZStencil());
	DEBUG_SET_PRIV_DATA(mGBuffer, "render_pipeline.gbuffer");

	mTempFbMng = CreateInstance<TempFrameBufferManager>(resMng, renderSys.WinSize(), MakeResFormats(kFormatR8G8B8A8UNorm, kFormatD24UNormS8UInt));

	coroutine::ExecuteTaskSync(resMng.GetSyncService(), [&]()->CoTask<bool> {
		MaterialLoadParam loadParam(MAT_MODEL);
		res::MaterialInstance material;
		CoAwait resMng.CreateMaterial(material, __LaunchAsync__, loadParam);

		mGBufferSprite = rend::Sprite::Create(__LaunchAsync__, resMng, material);
		mGBufferSprite->SetPosition(Eigen::Vector3f(-1, -1, 0));
		mGBufferSprite->SetSize(Eigen::Vector3f(2, 2, 0));
		CoReturn true;
	}());
}

void RenderPipeline::BindPass(const res::PassPtr& pass)
{
	mRenderSys.SetProgram(pass->mProgram);
	mRenderSys.SetVertexLayout(pass->mInputLayout);

	if (!pass->mSamplers.empty()) mRenderSys.SetSamplers(0, &pass->mSamplers[0], pass->mSamplers.size());
	else mRenderSys.SetSamplers(0, nullptr, 0);
}

void RenderPipeline::RenderPass(const res::PassPtr& pass, const TextureVector& textures, int iterCnt, const RenderOperation& op)
{
	//auto lock = mStatesBlock.LockFrameBuffer(IF_OR(pass->mFrameBuffer, nullptr));

	if (textures.Count() > 0) mRenderSys.SetTextures(kTextureUserSlotFirst, &textures[0], textures.Count());
	else mRenderSys.SetTextures(kTextureUserSlotFirst, nullptr, 0);

	BindPass(pass);

	if (op.IndexBuffer) mRenderSys.DrawIndexedPrimitive(op, pass->mTopoLogy);
	else mRenderSys.DrawPrimitive(op, pass->mTopoLogy);
}

void RenderPipeline::RenderOp(const RenderOperation& op, const std::string& lightMode)
{
	op.Material.FlushGpuParameters(mRenderSys);

	std::map<std::string, IFrameBufferPtr> mGrabDic;

	res::TechniquePtr tech = op.Material->GetShader()->CurTech();
	std::vector<res::PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes) {
		if (pass->mGrabOutput) {
			IFrameBufferPtr passFb = mTempFbMng->Borrow();
			mGrabDic[pass->mGrabOutput.Name] = passFb;
			mStatesBlock.FrameBuffer.Push(passFb);
		}
		if (pass->mGrabInput) {
			IFrameBufferPtr passFb = mGrabDic[pass->mGrabInput.Name];
			if (passFb) mRenderSys.SetTexture(pass->mGrabInput.TextureSlot, passFb->GetAttachColorTexture(0));
		}
		
		//SetVertexLayout(pass->mInputLayout);
		mRenderSys.SetVertexBuffers(op.VertexBuffers);
		mRenderSys.SetIndexBuffer(op.IndexBuffer);
		mRenderSys.SetConstBuffers(op.Material.GetConstBuffers(), pass->mProgram);
		const TextureVector& textures = op.Material->GetTextures();
		RenderPass(pass, textures, -1, op);

		if (pass->mGrabOutput) {
			mStatesBlock.FrameBuffer.Pop();
		}
	}
}

void RenderPipeline::RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, unsigned camMask, const cbPerLight* lightParam, cbPerFrame& globalParam)
{
	for (const auto& op : opQueue) {
		auto shader = op.Material->GetShader();
		if ((op.CameraMask & camMask) && shader->IsLoaded()) {
			globalParam.World = op.WorldTransform;

			op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(globalParam));
			if (lightParam) op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*lightParam));

			RenderOp(op, lightMode);
		}
	}
}

static cbPerLight MakePerLight(const scene::Light& light)
{
	return light.MakeCbLight();
}
static cbPerFrame& MakePerFrame(cbPerFrame& perFrameParam, const Eigen::Vector3f& camPos, const Eigen::Vector2i& fbSize)
{
	perFrameParam.ViewInv = perFrameParam.View.inverse();
	perFrameParam.ProjectionInv = perFrameParam.Projection.inverse();

	perFrameParam.CameraPosition.head<3>() = camPos;
	perFrameParam.FrameBufferSize = Eigen::Vector4f(fbSize.x(), fbSize.y(), 1.0f / fbSize.x(), 1.0f / fbSize.y());
	return perFrameParam;
}
static cbPerFrame MakePerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize)
{
	cbPerFrame perFrameParam;
	perFrameParam.View = camera.GetView();
	perFrameParam.Projection = camera.GetProjection();
	return MakePerFrame(perFrameParam, camera.GetTransform()->GetLocalPosition(), fbSize);
}
static cbPerFrame MakeReceiveShadowPerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize, const scene::Light& light, IFrameBufferPtr shadowMap)
{
	cbPerFrame perFrameParam = MakePerFrame(camera, fbSize);
	if (shadowMap) {
		constexpr bool castShadow = false;
		light.CalculateLightingViewProjection(camera, fbSize, castShadow, perFrameParam.LightView, perFrameParam.LightProjection);
		MIR_TEST_CASE(CompareLightCameraByViewProjection(light, camera, fbSize, {}));
		perFrameParam.ShadowMapSize = Eigen::Vector4f(shadowMap->GetWidth(), shadowMap->GetHeight(), 1.0 / shadowMap->GetWidth(), 1.0 / shadowMap->GetHeight());
	}
	return perFrameParam;
}
static cbPerFrame MakeCastShadowPerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize, const scene::Light& light)
{
	cbPerFrame perFrameParam;
	{
		constexpr bool castShadow = true;
		light.CalculateLightingViewProjection(camera, fbSize, castShadow, perFrameParam.View, perFrameParam.Projection);
		MIR_TEST_CASE(CompareLightCameraByViewProjection(light, camera, fbSize, {}));
	}
	return MakePerFrame(perFrameParam, camera.GetTransform()->GetLocalPosition(), fbSize);
}
void RenderPipeline::RenderCameraForward(const RenderOperationQueue& opQueue, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights)
{
	if (lights.empty()) return;

	//LIGHTMODE_SHADOW_CASTER
	bool genSM = false;
	if (!opQueue.IsEmpty())
	{
	#if !defined DEBUG_SHADOW_CASTER
		auto fb_shadow_map = mStatesBlock.LockFrameBuffer(mShadowMap, Eigen::Vector4f::Zero(), 1.0, 0);
	#endif
		auto depth_state = mStatesBlock.LockDepth(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
		auto blend_state = mStatesBlock.LockBlend(BlendState::MakeDisable());
		for (auto& light : lights) {
			if ((light->GetCameraMask() & camera.GetCullingMask()) && (light->GetType() == kLightDirectional)) {
				genSM = true;
				cbPerFrame perFrame = MakeCastShadowPerFrame(camera, mRenderSys.WinSize(), *light);
				RenderLight(opQueue, LIGHTMODE_SHADOW_CASTER, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
				break;
			}
		}
	}
	
#if !defined DEBUG_SHADOW_CASTER
	//LIGHTMODE_FORWARD_BASE
	{
		auto fb_2post_process = mStatesBlock.LockFrameBuffer(IF_OR(camera.GetPostProcessInput(), nullptr), 
			Eigen::Vector4f::Zero(), 1.0, 0);
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();
		
		auto tex_shadow	= mStatesBlock.LockTexture(kPipeTextureShadowMap, IF_AND_OR(genSM, mShadowMap->GetAttachZStencilTexture(), nullptr));
		
		auto tex_spec_env = mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(camera.GetSkyBox(), GetTexture()));
		auto tex_diffuse_env = mStatesBlock.LockTexture(kPipeTextureDiffuseEnv, NULLABLE(camera.GetSkyBox(), GetDiffuseEnvMap()));
		auto tex_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(camera.GetSkyBox(), GetLutMap()));

		scene::LightPtr firstLight = nullptr;
		cbPerFrame perFrame;
		for (auto& light : lights) 
		{
			if (light->GetCameraMask() & camera.GetCullingMask()) 
			{
				if (firstLight == nullptr) 
				{
					firstLight = light;

					depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
					blend_state(BlendState::MakeAlphaNonPremultiplied());
					perFrame = MakeReceiveShadowPerFrame(camera, mRenderSys.WinSize(), *firstLight, IF_AND_OR(genSM, mShadowMap, nullptr));
					RenderLight(opQueue, LIGHTMODE_FORWARD_BASE, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
					
					if (auto skybox = camera.GetSkyBox()) {
						depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskAll));
						blend_state(BlendState::MakeAlphaNonPremultiplied());
						RenderOperationQueue opQue;
						skybox->GenRenderOperation(opQue);
						RenderLight(opQue, LIGHTMODE_FORWARD_BASE, camera.GetCullingMask(), nullptr, perFrame);
					}
				}
				else {
					depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskZero));
					blend_state(BlendState::MakeAdditive());
					RenderLight(opQueue, LIGHTMODE_FORWARD_ADD, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
				}
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights
	}
#endif
}

void RenderPipeline::RenderCameraDeffered(const RenderOperationQueue& opQueue, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights)
{
	if (lights.empty()) return;

	if (camera.GetPostProcessInput())
		mStatesBlock.FrameBuffer.Push(camera.GetPostProcessInput());

	//LIGHTMODE_PREPASS_BASE
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();
		mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);

		scene::LightPtr firstLight = nullptr;
		cbPerFrame perFrame;
		for (auto& light : lights) 
		{
			if (light->GetCameraMask() & camera.GetCullingMask()) 
			{
				if (firstLight == nullptr) 
				{
					firstLight = light;
					
					auto fb_gbuffer = mStatesBlock.LockFrameBuffer(mGBuffer);
					mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
					blend_state(BlendState::MakeDisable());
					depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));

					perFrame = MakeReceiveShadowPerFrame(camera, mRenderSys.WinSize(), *firstLight, mGBuffer);
					RenderLight(opQueue, LIGHTMODE_PREPASS_BASE, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
					
					blend_state(BlendState::MakeAlphaNonPremultiplied());
				}
				else {
					blend_state(BlendState::MakeAdditive());
				}
				depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));

				auto tex_shadow	= mStatesBlock.LockTexture(kPipeTextureShadowMap, mGBuffer->GetAttachZStencilTexture());
				
				auto tex_spec_env = mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(camera.GetSkyBox(), GetTexture()));
				auto tex_diffuse_env = mStatesBlock.LockTexture(kPipeTextureDiffuseEnv, NULLABLE(camera.GetSkyBox(), GetDiffuseEnvMap()));
				auto tex_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(camera.GetSkyBox(), GetLutMap()));

				auto tex_gpos	= mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
				auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));
				auto tex_galbedo = mStatesBlock.LockTexture(kPipeTextureGBufferAlbedo, mGBuffer->GetAttachColorTexture(2));
				auto tex_gemissive = mStatesBlock.LockTexture(kPipeTextureGBufferEmissive, mGBuffer->GetAttachColorTexture(3));
				RenderOperationQueue opQue;
				mGBufferSprite->GenRenderOperation(opQue);
				RenderLight(opQue, LIGHTMODE_PREPASS_FINAL, -1, &MakePerLight(*light), perFrame);
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights

		if (auto skybox = camera.GetSkyBox()) {
			blend_state(BlendState::MakeDisable());
			depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskZero));

			RenderOperationQueue opQue;
			skybox->GenRenderOperation(opQue);
			cbPerFrame perFrame = MakePerFrame(camera, mRenderSys.WinSize());
			RenderLight(opQue, LIGHTMODE_FORWARD_BASE, camera.GetCullingMask(), nullptr, perFrame);
		}
	}

	//LIGHTMODE_OVERLAY
	{
		auto blend_state = mStatesBlock.LockBlend(BlendState::MakeAlphaNonPremultiplied());
		auto depth_state = mStatesBlock.LockDepth(DepthState::MakeFor3D(false));

		cbPerFrame perFrame = MakePerFrame(camera, mRenderSys.WinSize());
		RenderLight(opQueue, LIGHTMODE_OVERLAY, camera.GetCullingMask(), nullptr, perFrame);
	}

	//LIGHTMODE_POSTPROCESS
	if (camera.GetPostProcessInput())
	{
		mStatesBlock.FrameBuffer.Pop();

		auto blend_state = mStatesBlock.LockBlend();
		auto depth_state = mStatesBlock.LockDepth();
		blend_state(BlendState::MakeDisable());
		depth_state(DepthState::MakeFor3D(false));

		auto postProcessInput = camera.GetPostProcessInput();
		auto& postProcessEffects = camera.GetPostProcessEffects();
		std::vector<IFrameBufferPtr> tempOutputs(postProcessEffects.size());
		for (size_t i = 0; i < tempOutputs.size() - 1; ++i)
			tempOutputs[i] = mTempFbMng->Borrow();
		tempOutputs.back() = camera.GetOutput();

		for (size_t i = 0; i < postProcessEffects.size(); ++i) {
			auto curfb = mStatesBlock.LockFrameBuffer(tempOutputs[i]);
			auto tex_input = mStatesBlock.LockTexture(kPipeTextureSceneImage, IF_AND_OR(i == 0, postProcessInput->GetAttachColorTexture(0), tempOutputs[i-1]->GetAttachColorTexture(0)));
			auto tex_shadow	= mStatesBlock.LockTexture(kPipeTextureShadowMap, mGBuffer->GetAttachZStencilTexture());
			auto tex_gpos = mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
			auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));

			RenderOperationQueue opQue;
			postProcessEffects[i]->GenRenderOperation(opQue);
			RenderLight(opQue, LIGHTMODE_POSTPROCESS, camera.GetCullingMask(), nullptr, MakePerFrame(camera, mRenderSys.WinSize()));
		}
	}
}

void RenderPipeline::Render(const RenderOperationQueue& opQueue, const std::vector<scene::CameraPtr>& cameras, const std::vector<scene::LightPtr>& lights)
{
	for (auto& camera : cameras) 
	{
		auto outLock = mStatesBlock.LockFrameBuffer(IF_OR(camera->GetOutput(), nullptr));
		//if (camera->GetOutput()) mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);

		if (camera->GetRenderingPath() == kRenderPathForward)
			RenderCameraForward(opQueue, *camera, lights);
		else
			RenderCameraDeffered(opQueue, *camera, lights);
	}
}

bool RenderPipeline::BeginFrame()
{
	return mRenderSys.BeginScene();
}
void RenderPipeline::EndFrame()
{
	mRenderSys.EndScene();
	mTempFbMng->ReturnAll();
}

}