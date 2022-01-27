#include "core/rendersys/render_pipeline.h"
#include "core/rendersys/render_states_block.h"
#include "core/resource/resource_manager.h"
#include "core/renderable/sprite.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/scene/scene_manager.h"
#include "core/scene/camera.h"
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/base/rendersys_debug.h"
#include "core/base/attribute_struct.h"
#include "test/unit_test/unit_test.h"

namespace mir {

enum PipeLineTextureSlot {
	kPipeTextureStart = 0,
	kPipeTextureMain = 0,
	kPipeTextureShadowMap = 8,
	kPipeTextureDiffuseEnv = 9,
	kPipeTextureSpecEnv = 10,
	kPipeTextureLUT = 11,
	kPipeTextureGBufferPos = 13,
	kPipeTextureGBufferNormal = 14,
	kPipeTextureGBufferAlbedo = 15
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
		MakeResFormats(kFormatR8G8B8A8UNorm,kFormatR8G8B8A8UNorm,kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt));
	DEBUG_SET_PRIV_DATA(mGBuffer, "render_pipeline.gbuffer");

	coroutine::ExecuteTaskSync(resMng.GetSyncService(), [&]()->cppcoro::shared_task<void> {
		MaterialLoadParam loadParam(MAT_MODEL);
		mGBufferSprite = co_await rend::Sprite::Create(__LaunchSync__, resMng, loadParam);
		mGBufferSprite->SetPosition(Eigen::Vector3f(-1, -1, 0));
		mGBufferSprite->SetSize(Eigen::Vector3f(2, 2, 0));
	}());
}

void RenderPipeline::BindPass(const res::PassPtr& pass)
{
	mRenderSys.SetProgram(pass->mProgram);

#if USE_MATERIAL_INSTANCE
#else
	auto cbuffers = pass->GetConstBuffers();
	mRenderSys.SetConstBuffers(0, &cbuffers[0], cbuffers.size(), pass->mProgram);
#endif

	mRenderSys.SetVertexLayout(pass->mInputLayout);

	if (!pass->mSamplers.empty()) mRenderSys.SetSamplers(0, &pass->mSamplers[0], pass->mSamplers.size());
	else mRenderSys.SetSamplers(0, nullptr, 0);
}

void RenderPipeline::RenderPass(const res::PassPtr& pass, const TextureVector& textures, int iterCnt, const RenderOperation& op)
{
	auto lock = mStatesBlock.LockFrameBuffer(IF_OR(pass->mFrameBuffer, nullptr));

	if (textures.Count() > 0) mRenderSys.SetTextures(kPipeTextureStart, &textures[0], textures.Count());
	else mRenderSys.SetTextures(kPipeTextureStart, nullptr, 0);

#if USE_MATERIAL_INSTANCE
	
#else
	for (auto& cbBytes : op.UBOBytesByName)
		pass->UpdateConstBufferByName(mRenderSys, cbBytes.first, Data::Make(cbBytes.second));
#endif

	BindPass(pass);

	if (op.IndexBuffer) mRenderSys.DrawIndexedPrimitive(op, pass->mTopoLogy);
	else mRenderSys.DrawPrimitive(op, pass->mTopoLogy);
}

void RenderPipeline::RenderOp(const RenderOperation& op, const std::string& lightMode)
{
#if USE_MATERIAL_INSTANCE
	res::TechniquePtr tech = op.Material->GetShader()->CurTech();
#else
	res::TechniquePtr tech = op.Shader->CurTech();
#endif

#if USE_MATERIAL_INSTANCE
	op.Material.FlushGpuParameters(mRenderSys);
#endif

	std::vector<res::PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes) {
		//SetVertexLayout(pass->mInputLayout);
		mRenderSys.SetVertexBuffers(op.VertexBuffers);
		mRenderSys.SetIndexBuffer(op.IndexBuffer);

	#if USE_MATERIAL_INSTANCE
		auto cbuffers = op.Material.GetConstBuffers();
		if (! cbuffers.empty()) mRenderSys.SetConstBuffers(0, &cbuffers[0], cbuffers.size(), pass->mProgram);
	#endif

	#if USE_MATERIAL_INSTANCE
		const TextureVector& textures = op.Material->GetTextures();
	#else
		const TextureVector& textures = op.Textures;
	#endif
		mRenderSys.SetVertexBuffers(op.VertexBuffers);
		
		RenderPass(pass, textures, -1, op);
	}
}

void RenderPipeline::RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, unsigned camMask, 
	const cbPerLight* lightParam, cbPerFrame& globalParam)
{
	for (const auto& op : opQueue) {
	#if USE_MATERIAL_INSTANCE
		auto shader = op.Material->GetShader();
	#else
		auto shader = op.Shader;
	#endif
		if ((op.CameraMask & camMask) && shader->IsLoaded()) {
			auto curTech = shader->CurTech();

			globalParam.World = op.WorldTransform;
		#if USE_MATERIAL_INSTANCE
			op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(globalParam));
			if (lightParam) op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*lightParam));
		#else
			curTech->UpdateConstBufferByName(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(globalParam));
			if (lightParam) curTech->UpdateConstBufferByName(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*lightParam));
		#endif

			RenderOp(op, lightMode);
		}
	}
}

static cbPerLight MakePerLight(const scene::ILight& light)
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
	return MakePerFrame(perFrameParam, camera.GetTransform()->GetPosition(), fbSize);
}
static cbPerFrame MakeReceiveShadowPerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize, const scene::ILight& light, IFrameBufferPtr shadowMap)
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
static cbPerFrame MakeCastShadowPerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize, const scene::ILight& light)
{
	cbPerFrame perFrameParam;
	{
		constexpr bool castShadow = true;
		light.CalculateLightingViewProjection(camera, fbSize, castShadow, perFrameParam.View, perFrameParam.Projection);
		MIR_TEST_CASE(CompareLightCameraByViewProjection(light, camera, fbSize, {}));
	}
	return MakePerFrame(perFrameParam, camera.GetTransform()->GetPosition(), fbSize);
}
void RenderPipeline::RenderCameraForward(const RenderOperationQueue& opQueue, const scene::Camera& camera,
	const std::vector<ILightPtr>& lights)
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
		auto fb_2post_process = mStatesBlock.LockFrameBuffer(IF_OR(camera.GetOutput2PostProcess(), nullptr), 
			Eigen::Vector4f::Zero(), 1.0, 0);
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();
		auto tex_shadow	= mStatesBlock.LockTexture(kPipeTextureShadowMap, IF_AND_OR(genSM, mShadowMap->GetAttachZStencilTexture(), nullptr));
		auto tex_spec_env = mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(camera.GetSkyBox(), GetTexture()));
		auto tex_diffuse_env = mStatesBlock.LockTexture(kPipeTextureDiffuseEnv, NULLABLE(camera.GetSkyBox(), GetDiffuseEnvMap()));
		auto tex_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(camera.GetSkyBox(), GetLutMap()));

		ILightPtr firstLight = nullptr;
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
	
	//LIGHTMODE_POSTPROCESS
	if (camera.GetOutput2PostProcess())
	{
		auto depth_state = mStatesBlock.LockDepth(DepthState::MakeFor3D(false));
		mStatesBlock.Textures(kPipeTextureMain, camera.GetOutput2PostProcess()->GetAttachColorTexture(0));

		RenderOperationQueue opQue;
		auto& postProcessEffects = camera.GetPostProcessEffects();
		for (size_t i = 0; i < postProcessEffects.size(); ++i)
			postProcessEffects[i]->GenRenderOperation(opQue);
		RenderLight(opQue, LIGHTMODE_POSTPROCESS, camera.GetCullingMask(), nullptr, MakePerFrame(camera, mRenderSys.WinSize()));
	}
#endif
}

void RenderPipeline::RenderCameraDeffered(const RenderOperationQueue& opQueue, const scene::Camera& camera, const std::vector<ILightPtr>& lights)
{
	if (lights.empty()) return;

	//LIGHTMODE_PREPASS_BASE
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		ILightPtr firstLight = nullptr;
		cbPerFrame perFrame;
		for (auto& light : lights) 
		{
			if (light->GetCameraMask() & camera.GetCullingMask()) 
			{
				if (firstLight == nullptr) 
				{
					firstLight = light;
					
					{
					#if !defined DEBUG_PREPASS_BASE
						auto fb_gbuffer = mStatesBlock.LockFrameBuffer(mGBuffer);
					#endif
						mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);

						depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
						blend_state(BlendState::MakeAlphaNonPremultiplied());
						perFrame = MakeReceiveShadowPerFrame(camera, mRenderSys.WinSize(), *firstLight, mGBuffer);
						RenderLight(opQueue, LIGHTMODE_PREPASS_BASE, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
						
						if (auto skybox = camera.GetSkyBox()) {
							depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskAll));
							blend_state(BlendState::MakeAlphaNonPremultiplied());
							RenderOperationQueue opQue;
							skybox->GenRenderOperation(opQue);
							RenderLight(opQue, LIGHTMODE_PREPASS_BASE, camera.GetCullingMask(), nullptr, perFrame);
						}
					}
				#if !defined DEBUG_PREPASS_BASE
					mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
				#endif
				}//firstLight == nullptr
				else {
					blend_state(BlendState::MakeAdditive());
				}

			#if !defined DEBUG_PREPASS_BASE
				depth_state(DepthState::MakeFor3D(false));
				auto tex_shadow	= mStatesBlock.LockTexture(kPipeTextureShadowMap, mGBuffer->GetAttachZStencilTexture());
				auto tex_env	= mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(camera.GetSkyBox(), GetTexture()));
				auto tex_gpos	= mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
				auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));
				auto tex_galbedo = mStatesBlock.LockTexture(kPipeTextureGBufferAlbedo, mGBuffer->GetAttachColorTexture(2));
				RenderOperationQueue opQue;
				mGBufferSprite->GenRenderOperation(opQue);
				RenderLight(opQue, LIGHTMODE_PREPASS_FINAL, -1, &MakePerLight(*light), perFrame);
			#endif
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights
	}
}

void RenderPipeline::Render(const RenderOperationQueue& opQueue, SceneManager& scene)
{
	for (auto& camera : scene.GetCameras()) 
	{
		auto outLock = mStatesBlock.LockFrameBuffer(IF_OR(camera->GetOutput(), nullptr));
		if (camera->GetOutput()) {
			mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
		}

		if (camera->GetRenderingPath() == kRenderPathForward)
			RenderCameraForward(opQueue, *camera, scene.GetLights());
		else
			RenderCameraDeffered(opQueue, *camera, scene.GetLights());
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

void RenderPipeline::Draw(IRenderable& rend, SceneManager& scene)
{
	if (BeginFrame()) {
		RenderOperationQueue opQue;
		rend.GenRenderOperation(opQue);
		Render(opQue, scene);
		EndFrame();
	}
}

}