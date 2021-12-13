#include "core/rendersys/render_pipeline.h"
#include "core/resource/resource_manager.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/scene/scene_manager.h"
#include "core/scene/camera.h"
#include "core/base/debug.h"
#include "core/base/macros.h"
#include "test/unit_test/unit_test.h"

//#define DEBUG_SHADOW_CASTER

namespace mir {

#define TEXTURE_SLOT_START 0
#define E_TEXTURE_MAIN 0
#define E_TEXTURE_DEPTH_MAP 8
#define E_TEXTURE_ENV 9

RenderPipeline::RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng)
	:mRenderSys(renderSys)
{
	mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, renderSys.WinSize(), MakeResFormats(kFormatUnknown, kFormatD24UNormS8UInt));
	DEBUG_SET_PRIV_DATA(mShadowMap, "render_pipeline.shadow_map");
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
			globalParam.WorldInv = globalParam.World.inverse();
			curTech->UpdateConstBufferByName(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(globalParam));
			
			if (lightParam) curTech->UpdateConstBufferByName(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*lightParam));

			RenderOp(op, lightMode);
		}
	}
}

static cbPerLight MakeCbPerLight(const ILight& light)
{
	cbPerLight lightParam = {};
	lightParam = light.MakeCbLight();
	lightParam.IsSpotLight = light.GetType() == kLightSpot;
	return lightParam;
}
static cbPerFrame MakeCbPerFrame(const Camera& camera) 
{
	cbPerFrame globalParam = {};
	globalParam.View = camera.GetView();
	globalParam.Projection = camera.GetProjection();

	globalParam.WorldInv = globalParam.World.inverse();
	globalParam.ViewInv = globalParam.View.inverse();
	globalParam.ProjectionInv = globalParam.Projection.inverse();
	return globalParam;
}
static cbPerFrame MakeCbPerFrame(const Camera& camera, const ILight& light, bool castShadow, IFrameBufferPtr shadowMap)
{
	cbPerFrame globalParam = {};
	if (castShadow) {
		light.CalculateLightingViewProjection(camera, castShadow, globalParam.View, globalParam.Projection);
	}
	else {
		globalParam.View = camera.GetView();
		globalParam.Projection = camera.GetProjection();
		light.CalculateLightingViewProjection(camera, false, globalParam.LightView, globalParam.LightProjection);
		MIR_TEST_CASE(CompareLightCameraByViewProjection(light, camera, {}));

		globalParam._ShadowMapTexture_TexelSize.head<2>() = shadowMap
			? Eigen::Vector2f(1.0 / shadowMap->GetWidth(), 1.0 / shadowMap->GetHeight())
			: Eigen::Vector2f::Zero();
	}

	MIR_TEST_CASE(
		TestViewProjectionWithCases(camera.GetView(), camera.GetProjection());
		Eigen::Matrix4f light_view, light_proj;
		light.CalculateLightingViewProjection(camera, false, light_view, light_proj);
		TestViewProjectionWithCases(light_view, light_proj);
	);

	globalParam.WorldInv = globalParam.World.inverse();
	globalParam.ViewInv = globalParam.View.inverse();
	globalParam.ProjectionInv = globalParam.Projection.inverse();
	return globalParam;
}

void RenderPipeline::RenderCamera(const RenderOperationQueue& opQueue, const Camera& camera, 
	const std::vector<ILightPtr>& lights)
{
	if (opQueue.IsEmpty() || lights.empty()) return;

	//E_PASS_SHADOWCASTER
	bool shadowMapGenerated = false;
	{
	#if !defined DEBUG_SHADOW_CASTER
		_PushFrameBuffer(mShadowMap);
	#endif
		mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
		DepthState orgDS = mRenderSys.GetDepthState();
		BlendState orgBS = mRenderSys.GetBlendFunc();
		mRenderSys.SetDepthState(DepthState::MakeFor3D(true));
		mRenderSys.SetBlendFunc(BlendState::MakeDisable());

		for (auto& light : lights) {
			if ((light->GetCameraMask() & camera.GetCameraMask()) && (light->GetType() == kLightDirectional)) {
				shadowMapGenerated = true;

				cbPerFrame globalParam = MakeCbPerFrame(camera, *light, true, nullptr);
				cbPerLight lightParam = MakeCbPerLight(*light);
				RenderLight(opQueue, E_PASS_SHADOWCASTER, camera.GetCameraMask(), &lightParam, globalParam);
				break;
			}
		}

	#if !defined DEBUG_SHADOW_CASTER
		_PopFrameBuffer();
	#endif
		mRenderSys.SetDepthState(orgDS);
		mRenderSys.SetBlendFunc(orgBS);

		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
	}
	
#if !defined DEBUG_SHADOW_CASTER
	//E_PASS_FORWARDBASE
	{
		if (camera.GetOutput2PostProcess()) {
			_PushFrameBuffer(camera.GetOutput2PostProcess());
			mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
		}

		BlendState orgBS = mRenderSys.GetBlendFunc();
		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, shadowMapGenerated ? mShadowMap->GetAttachZStencilTexture() : nullptr);
		mRenderSys.SetTexture(E_TEXTURE_ENV, NULLABLE(camera.GetSkyBox(), GetTexture()));
			
		ILightPtr firstLight = nullptr;
		cbPerFrame globalParam;
		for (auto& light : lights) {
			if (light->GetCameraMask() & camera.GetCameraMask()) {
				if (firstLight == nullptr) {
					firstLight = light;
					globalParam = MakeCbPerFrame(camera, *firstLight, false, shadowMapGenerated ? mShadowMap : nullptr);

					mRenderSys.SetBlendFunc(BlendState::MakeAlphaNonPremultiplied());
					if (auto skybox = camera.GetSkyBox()) {
						RenderOperationQueue opQue;
						skybox->GenRenderOperation(opQue);
						RenderLight(opQue, E_PASS_FORWARDBASE, camera.GetCameraMask(), nullptr, globalParam);
					}
					auto lightParam = MakeCbPerLight(*light);
					RenderLight(opQueue, E_PASS_FORWARDBASE, camera.GetCameraMask(), &lightParam, globalParam);
				}
				else {
					mRenderSys.SetBlendFunc(BlendState::MakeAdditive());
					auto lightParam = MakeCbPerLight(*light);
					RenderLight(opQueue, E_PASS_FORWARDADD, camera.GetCameraMask(), &lightParam, globalParam);
				}
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights

		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
		mRenderSys.SetTexture(E_TEXTURE_ENV, nullptr);
		mRenderSys.SetBlendFunc(orgBS);
	}
	
	//E_PASS_POSTPROCESS
	if (camera.GetOutput2PostProcess())
	{
		DepthState orgDS = mRenderSys.GetDepthState();
		mRenderSys.SetDepthState(DepthState::MakeFor3D(false));
		mRenderSys.SetTexture(E_TEXTURE_MAIN, camera.GetOutput2PostProcess()->GetAttachColorTexture(0));

		RenderOperationQueue opQue;
		auto& postProcessEffects = camera.GetPostProcessEffects();
		for (size_t i = 0; i < postProcessEffects.size(); ++i)
			postProcessEffects[i]->GenRenderOperation(opQue);
		RenderLight(opQue, E_PASS_POSTPROCESS, camera.GetCameraMask(), nullptr, MakeCbPerFrame(camera));

		mRenderSys.SetTexture(E_TEXTURE_MAIN, nullptr);
		mRenderSys.SetDepthState(orgDS);
	}
#endif
}

void RenderPipeline::Render(const RenderOperationQueue& opQueue, SceneManager& scene)
{
	for (auto& camera : scene.GetCameras()) 
	{
		if (camera->GetOutput()) {
			_PushFrameBuffer(camera->GetOutput());
			mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
		}

		RenderCamera(opQueue, *camera, scene.GetLights());

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