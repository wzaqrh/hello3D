#include "core/rendersys/render_pipeline.h"
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

#define TEXTURE_SLOT_START 0
#define TEXTURE_MAIN 0
#define TEXTURE_SHADOW_MAP 8
#define TEXTURE_ENVIROMENT 9

#define TEXTURE_GBUFFER_POS 10
#define TEXTURE_GBUFFER_NORMAL 11
#define TEXTURE_GBUFFER_ALBEDO 12

RenderPipeline::RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng)
	:mRenderSys(renderSys)
{
	mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, renderSys.WinSize(), 
		MakeResFormats(kFormatUnknown, kFormatD24UNormS8UInt));
	DEBUG_SET_PRIV_DATA(mShadowMap, "render_pipeline.shadow_map");

	mGBuffer = resMng.CreateFrameBuffer(__LaunchSync__, renderSys.WinSize(), 
		MakeResFormats(kFormatR8G8B8A8UNorm,kFormatR8G8B8A8UNorm,kFormatR8G8B8A8UNorm,kFormatD24UNormS8UInt));
	DEBUG_SET_PRIV_DATA(mGBuffer, "render_pipeline.gbuffer");

	mGBufferSprite = Sprite::Create(__LaunchSync__, resMng, MAT_MODEL);
	mGBufferSprite->SetPosition(Eigen::Vector3f(-1, -1, 0));
	mGBufferSprite->SetSize(Eigen::Vector3f(2, 2, 0));
}

void RenderPipeline::_PushFrameBuffer(IFrameBufferPtr rendTarget)
{
	mFrameBufferStack.push_back(rendTarget);
	mRenderSys.SetFrameBuffer(rendTarget);
}
void RenderPipeline::_PopFrameBuffer()
{
	BOOST_ASSERT(!mFrameBufferStack.empty());
	mFrameBufferStack.pop_back();

	mRenderSys.SetFrameBuffer(!mFrameBufferStack.empty() ? mFrameBufferStack.back() : nullptr);
}

void RenderPipeline::BindPass(const PassPtr& pass)
{
	mRenderSys.SetProgram(pass->mProgram);

	auto cbuffers = pass->GetConstBuffers();
	mRenderSys.SetConstBuffers(0, &cbuffers[0], cbuffers.size(), pass->mProgram);

	mRenderSys.SetVertexLayout(pass->mInputLayout);

	if (!pass->mSamplers.empty()) mRenderSys.SetSamplers(0, &pass->mSamplers[0], pass->mSamplers.size());
	else mRenderSys.SetSamplers(0, nullptr, 0);
}

void RenderPipeline::RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op)
{
	if (iterCnt >= 0) _PushFrameBuffer(pass->mRTIterators[iterCnt]);
	else if (pass->mRenderTarget) _PushFrameBuffer(pass->mRenderTarget);

	if (iterCnt >= 0) {
		if (iterCnt + 1 < pass->mRTIterators.size())
			textures[0] = pass->mRTIterators[iterCnt + 1]->GetAttachColorTexture(0);
	}
	else {
		if (!pass->mRTIterators.empty())
			textures[0] = pass->mRTIterators[0]->GetAttachColorTexture(0);
	}

	{
		//if (op.OnBind) op.OnBind(mRenderSys, *pass, op);

		if (textures.Count() > 0) mRenderSys.SetTextures(TEXTURE_SLOT_START, &textures.Textures[0], textures.Textures.size());
		else mRenderSys.SetTextures(TEXTURE_SLOT_START, nullptr, 0);
		
		for (auto& cbBytes : op.UBOBytesByName)
			pass->UpdateConstBufferByName(mRenderSys, cbBytes.first, Data::Make(cbBytes.second));

		BindPass(pass);

		if (op.IndexBuffer) mRenderSys.DrawIndexedPrimitive(op, pass->mTopoLogy);
		else mRenderSys.DrawPrimitive(op, pass->mTopoLogy);

		//if (op.OnUnbind) op.OnUnbind(mRenderSys, *pass, op);
	}

	if (iterCnt >= 0) {
		_PopFrameBuffer();
	}
	else {
		if (pass->mRenderTarget)
			_PopFrameBuffer();
	}
}

void RenderPipeline::RenderOp(const RenderOperation& op, const std::string& lightMode)
{
	TechniquePtr tech = op.Material->CurTech();
	std::vector<PassPtr> passes = tech->GetPassesByLightMode(lightMode);
	for (auto& pass : passes) {
		//SetVertexLayout(pass->mInputLayout);
		mRenderSys.SetVertexBuffers(op.VertexBuffers);
		mRenderSys.SetIndexBuffer(op.IndexBuffer);

		TextureBySlot textures = op.Textures;
		textures.Merge(pass->mTextures);

		for (int i = pass->mRTIterators.size() - 1; i >= 0; --i) {
			auto iter = op.VertBufferByPass.find(std::make_pair(pass, i));
			if (iter != op.VertBufferByPass.end()) mRenderSys.SetVertexBuffer(iter->second);
			else mRenderSys.SetVertexBuffers(op.VertexBuffers);
			
			ITexturePtr first = !textures.Empty() ? textures[0] : nullptr;
			RenderPass(pass, textures, i, op);
			textures[0] = first;
		}
		auto iter = op.VertBufferByPass.find(std::make_pair(pass, -1));
		if (iter != op.VertBufferByPass.end()) mRenderSys.SetVertexBuffer(iter->second);
		else mRenderSys.SetVertexBuffers(op.VertexBuffers);
		
		RenderPass(pass, textures, -1, op);
	}
}

void RenderPipeline::RenderLight(const RenderOperationQueue& opQueue, const std::string& lightMode, unsigned camMask, 
	const cbPerLight* lightParam, cbPerFrame& globalParam)
{
	for (const auto& op : opQueue) {
		if ((op.CameraMask & camMask) && op.Material->IsLoaded()) {
			auto curTech = op.Material->CurTech();

			globalParam.World = op.WorldTransform;
			curTech->UpdateConstBufferByName(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(globalParam));
			
			if (lightParam) curTech->UpdateConstBufferByName(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*lightParam));

			RenderOp(op, lightMode);
		}
	}
}

static cbPerLight MakePerLight(const ILight& light)
{
	return light.MakeCbLight();
}
static cbPerFrame MakePerFrame(const Camera& camera) 
{
	cbPerFrame perFrameParam = {};
	perFrameParam.View = camera.GetView();
	perFrameParam.Projection = camera.GetProjection();

	perFrameParam.ViewInv = perFrameParam.View.inverse();
	perFrameParam.ProjectionInv = perFrameParam.Projection.inverse();
	return perFrameParam;
}
static cbPerFrame MakeReceiveShadowPerFrame(const Camera& camera, const ILight& light, Eigen::Vector2i size, IFrameBufferPtr shadowMap)
{
	cbPerFrame perFrameParam = {};
	perFrameParam.View = camera.GetView();
	perFrameParam.Projection = camera.GetProjection();
	if (shadowMap) {
		constexpr bool castShadow = false;
		light.CalculateLightingViewProjection(camera, size, castShadow, perFrameParam.LightView, perFrameParam.LightProjection);
		MIR_TEST_CASE(CompareLightCameraByViewProjection(light, camera, size, {}));
		perFrameParam._ShadowMapTexture_TexelSize.head<2>() = Eigen::Vector2f(1.0 / shadowMap->GetWidth(), 1.0 / shadowMap->GetHeight());
	}
	return perFrameParam;
}
static cbPerFrame MakeCastShadowPerFrame(const Camera& camera, const ILight& light, Eigen::Vector2i size)
{
	cbPerFrame perFrameParam = {};
	constexpr bool castShadow = true;
	light.CalculateLightingViewProjection(camera, size, castShadow, perFrameParam.View, perFrameParam.Projection);
	MIR_TEST_CASE(CompareLightCameraByViewProjection(light, camera, size, {}));

	perFrameParam.ViewInv = perFrameParam.View.inverse();
	perFrameParam.ProjectionInv = perFrameParam.Projection.inverse();
	return perFrameParam;
}
void RenderPipeline::RenderCameraForward(const RenderOperationQueue& opQueue, const Camera& camera, 
	const std::vector<ILightPtr>& lights)
{
	if (lights.empty()) return;

	//LIGHTMODE_SHADOW_CASTER
	bool shadowMapGenerated = false;
	if (!opQueue.IsEmpty())
	{
	#if !defined DEBUG_SHADOW_CASTER
		_PushFrameBuffer(mShadowMap);
	#endif
		mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
		DepthState orgDS = mRenderSys.GetDepthState();
		BlendState orgBS = mRenderSys.GetBlendFunc();
		mRenderSys.SetDepthState(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
		mRenderSys.SetBlendFunc(BlendState::MakeDisable());

		for (auto& light : lights) {
			if ((light->GetCameraMask() & camera.GetCullingMask()) && (light->GetType() == kLightDirectional)) {
				shadowMapGenerated = true;

				cbPerFrame perFrame = MakeCastShadowPerFrame(camera, *light, mRenderSys.WinSize());
				RenderLight(opQueue, LIGHTMODE_SHADOW_CASTER, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
				break;
			}
		}

	#if !defined DEBUG_SHADOW_CASTER
		_PopFrameBuffer();
	#endif
		mRenderSys.SetDepthState(orgDS);
		mRenderSys.SetBlendFunc(orgBS);

		mRenderSys.SetTexture(TEXTURE_SHADOW_MAP, nullptr);
	}
	
#if !defined DEBUG_SHADOW_CASTER
	//LIGHTMODE_FORWARD_BASE
	{
		if (camera.GetOutput2PostProcess()) {
			_PushFrameBuffer(camera.GetOutput2PostProcess());
		}
		mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);

		DepthState orgDS = mRenderSys.GetDepthState();
		BlendState orgBS = mRenderSys.GetBlendFunc();
		mRenderSys.SetTexture(TEXTURE_SHADOW_MAP, shadowMapGenerated ? mShadowMap->GetAttachZStencilTexture() : nullptr);
		mRenderSys.SetTexture(TEXTURE_ENVIROMENT, NULLABLE(camera.GetSkyBox(), GetTexture()));
			
		ILightPtr firstLight = nullptr;
		cbPerFrame perFrame;
		for (auto& light : lights) {
			if (light->GetCameraMask() & camera.GetCullingMask()) {
				if (firstLight == nullptr) {
					firstLight = light;
					perFrame = MakeReceiveShadowPerFrame(camera, *firstLight, mRenderSys.WinSize(), shadowMapGenerated ? mShadowMap : nullptr);

					mRenderSys.SetDepthState(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
					mRenderSys.SetBlendFunc(BlendState::MakeAlphaNonPremultiplied());
					RenderLight(opQueue, LIGHTMODE_FORWARD_BASE, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
					
					if (auto skybox = camera.GetSkyBox()) {
						mRenderSys.SetDepthState(DepthState::Make(kCompareLessEqual, kDepthWriteMaskAll));
						mRenderSys.SetBlendFunc(BlendState::MakeAlphaNonPremultiplied());
						RenderOperationQueue opQue;
						skybox->GenRenderOperation(opQue);
						RenderLight(opQue, LIGHTMODE_FORWARD_BASE, camera.GetCullingMask(), nullptr, perFrame);
					}
				}
				else {
					mRenderSys.SetDepthState(DepthState::Make(kCompareLessEqual, kDepthWriteMaskZero));
					mRenderSys.SetBlendFunc(BlendState::MakeAdditive());
					RenderLight(opQueue, LIGHTMODE_FORWARD_ADD, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
				}
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights

		mRenderSys.SetTexture(TEXTURE_SHADOW_MAP, nullptr);
		mRenderSys.SetTexture(TEXTURE_ENVIROMENT, nullptr);
		mRenderSys.SetDepthState(orgDS);
		mRenderSys.SetBlendFunc(orgBS);
	}
	
	//LIGHTMODE_POSTPROCESS
	if (camera.GetOutput2PostProcess())
	{
		DepthState orgDS = mRenderSys.GetDepthState();
		mRenderSys.SetDepthState(DepthState::MakeFor3D(false));
		mRenderSys.SetTexture(TEXTURE_MAIN, camera.GetOutput2PostProcess()->GetAttachColorTexture(0));

		RenderOperationQueue opQue;
		auto& postProcessEffects = camera.GetPostProcessEffects();
		for (size_t i = 0; i < postProcessEffects.size(); ++i)
			postProcessEffects[i]->GenRenderOperation(opQue);
		RenderLight(opQue, LIGHTMODE_POSTPROCESS, camera.GetCullingMask(), nullptr, MakePerFrame(camera));

		mRenderSys.SetTexture(TEXTURE_MAIN, nullptr);
		mRenderSys.SetDepthState(orgDS);
	}
#endif
}

void RenderPipeline::RenderCameraDeffered(const RenderOperationQueue& opQueue, const Camera& camera, const std::vector<ILightPtr>& lights)
{
	if (lights.empty()) return;

	//LIGHTMODE_PREPASS_BASE
	{
		DepthState orgDS = mRenderSys.GetDepthState();
		BlendState orgBS = mRenderSys.GetBlendFunc();
		ILightPtr firstLight = nullptr;
		cbPerFrame perFrame;
		for (auto& light : lights) {
			if (light->GetCameraMask() & camera.GetCullingMask()) {
				if (firstLight == nullptr) {
					firstLight = light;
					
				#if !defined DEBUG_PREPASS_BASE
					_PushFrameBuffer(mGBuffer);
				#endif
					mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
					{
						mRenderSys.SetDepthState(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
						mRenderSys.SetBlendFunc(BlendState::MakeAlphaNonPremultiplied());
						perFrame = MakeReceiveShadowPerFrame(camera, *firstLight, mRenderSys.WinSize(), mGBuffer);
						RenderLight(opQueue, LIGHTMODE_PREPASS_BASE, camera.GetCullingMask(), &MakePerLight(*light), perFrame);
						
						if (auto skybox = camera.GetSkyBox()) {
							mRenderSys.SetDepthState(DepthState::Make(kCompareLessEqual, kDepthWriteMaskAll));
							mRenderSys.SetBlendFunc(BlendState::MakeAlphaNonPremultiplied());
							RenderOperationQueue opQue;
							skybox->GenRenderOperation(opQue);
							RenderLight(opQue, LIGHTMODE_PREPASS_BASE, camera.GetCullingMask(), nullptr, MakePerFrame(camera));
						}
					}
				#if !defined DEBUG_PREPASS_BASE
					_PopFrameBuffer();

					perFrame = MakeReceiveShadowPerFrame(camera, *firstLight, mRenderSys.WinSize(), mGBuffer);
					mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
				#endif
				}//firstLight == nullptr
				else {
					mRenderSys.SetBlendFunc(BlendState::MakeAdditive());
				}

			#if !defined DEBUG_PREPASS_BASE
				mRenderSys.SetDepthState(DepthState::MakeFor3D(false));
				mRenderSys.SetTexture(TEXTURE_ENVIROMENT, NULLABLE(camera.GetSkyBox(), GetTexture()));
				mRenderSys.SetTexture(TEXTURE_SHADOW_MAP, mGBuffer->GetAttachZStencilTexture());
				mRenderSys.SetTexture(TEXTURE_GBUFFER_POS, mGBuffer->GetAttachColorTexture(0));
				mRenderSys.SetTexture(TEXTURE_GBUFFER_NORMAL, mGBuffer->GetAttachColorTexture(1));
				mRenderSys.SetTexture(TEXTURE_GBUFFER_ALBEDO, mGBuffer->GetAttachColorTexture(2));
				
				RenderOperationQueue opQue;
				mGBufferSprite->GenRenderOperation(opQue);
				RenderLight(opQue, LIGHTMODE_PREPASS_FINAL, -1, &MakePerLight(*light), perFrame);

				mRenderSys.SetTexture(TEXTURE_ENVIROMENT, nullptr);
				mRenderSys.SetTexture(TEXTURE_SHADOW_MAP, nullptr);
				mRenderSys.SetTexture(TEXTURE_GBUFFER_POS, nullptr);
				mRenderSys.SetTexture(TEXTURE_GBUFFER_NORMAL, nullptr);
				mRenderSys.SetTexture(TEXTURE_GBUFFER_ALBEDO, nullptr);
			#endif
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights

		mRenderSys.SetTexture(TEXTURE_SHADOW_MAP, nullptr);
		mRenderSys.SetTexture(TEXTURE_ENVIROMENT, nullptr);
		mRenderSys.SetDepthState(orgDS);
		mRenderSys.SetBlendFunc(orgBS);
	}
}

void RenderPipeline::Render(const RenderOperationQueue& opQueue, SceneManager& scene)
{
	for (auto& camera : scene.GetCameras()) 
	{
		if (camera->GetOutput()) {
			_PushFrameBuffer(camera->GetOutput());
			mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
		}

		if (camera->GetRenderingPath() == kRenderPathForward)
			RenderCameraForward(opQueue, *camera, scene.GetLights());
		else
			RenderCameraDeffered(opQueue, *camera, scene.GetLights());

		if (camera->GetOutput()) {
			_PopFrameBuffer();
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