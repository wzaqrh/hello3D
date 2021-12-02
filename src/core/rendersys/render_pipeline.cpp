#include "core/rendersys/render_pipeline.h"
#include "core/resource/resource_manager.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/scene/scene_manager.h"
#include "core/scene/camera.h"
#include "core/base/debug.h"
#include "test/unit_test/unit_test.h"

//#define DEBUG_SHADOW_CASTER

namespace mir {

#define E_TEXTURE_MAIN 0
#define E_TEXTURE_DEPTH_MAP 8
#define E_TEXTURE_ENV 9

RenderPipeline::RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, const Eigen::Vector2i& size)
	:mRenderSys(renderSys)
	,mScreenSize(size)
{
	mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, mScreenSize, MakeResFormats(kFormatUnknown, kFormatD24UNormS8UInt));
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

	if (!pass->mSamplers.empty())
		mRenderSys.SetSamplers(0, &pass->mSamplers[0], pass->mSamplers.size());
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
		if (textures.Count() > 0)
			mRenderSys.SetTextures(E_TEXTURE_MAIN, &textures.Textures[0], textures.Textures.size());

		//if (op.OnBind) op.OnBind(mRenderSys, *pass, op);

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
	const cbPerLight& lightParam, cbGlobalParam& globalParam)
{
	for (int i = 0; i < opQueue.Count(); ++i) {
		auto& op = opQueue[i];
		if ((op.CameraMask & camMask) && op.Material->IsLoaded()) {
			globalParam.World = op.WorldTransform;
			globalParam.WorldInv = globalParam.World.inverse();
			op.Material->CurTech()->UpdateConstBufferByName(mRenderSys, 
				MAKE_CBNAME(cbGlobalParam), Data::Make(globalParam));
			
			op.Material->CurTech()->UpdateConstBufferByName(mRenderSys,
				MAKE_CBNAME(cbPerLight), Data::Make(lightParam));

			RenderOp(op, lightMode);
		}
	}
}

static std::tuple<cbGlobalParam, cbPerLight> MakeAutoParam(const Camera& camera, bool castShadow, const ILight& light)
{
	cbGlobalParam globalParam = {};
	cbPerLight lightParam = {};

	lightParam = light.MakeCbLight();
	lightParam.HasDepthMap = castShadow ? TRUE : FALSE;
	lightParam.LightType = light.GetType() + 1;

	if (castShadow) {
		light.CalculateLightingViewProjection(camera, globalParam.View, globalParam.Projection);
	}
	else {
		globalParam.View = camera.GetView();
		globalParam.Projection = camera.GetProjection();
		light.CalculateLightingViewProjection(camera, lightParam.LightView, lightParam.LightProjection);
		MIR_TEST_CASE(CompareLightCameraByViewProjection(light, camera, {}));
	}

	MIR_TEST_CASE(
		TestViewProjectionWithCases(camera.GetView(), camera.GetProjection());
		Eigen::Matrix4f light_view, light_proj;
		light.CalculateLightingViewProjection(camera, light_view, light_proj);
		TestViewProjectionWithCases(light_view, light_proj);
	);

	globalParam.WorldInv = globalParam.World.inverse();
	globalParam.ViewInv = globalParam.View.inverse();
	globalParam.ProjectionInv = globalParam.Projection.inverse();
	
	return std::tie(globalParam, lightParam);
}


void RenderPipeline::RenderOpQueue(const RenderOperationQueue& opQueue, const Camera& camera, 
	const std::vector<ILightPtr>& lightsOrder, const std::string& lightMode)
{
	if (opQueue.IsEmpty()) return;

	DepthState orgDS = mRenderSys.GetDepthState();
	BlendState orgBS = mRenderSys.GetBlendFunc();

	if (lightMode == E_PASS_SHADOWCASTER) {
	#if !defined DEBUG_SHADOW_CASTER
		_PushFrameBuffer(mShadowMap);
	#endif
		mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f(0,0,0,0), 1.0, 0);
		mRenderSys.SetDepthState(DepthState::MakeFor3D(true));
		mRenderSys.SetBlendFunc(BlendState::MakeDisable());
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, mShadowMap->GetAttachZStencilTexture());

		auto& skyBox = camera.GetSkyBox();
		if (skyBox && skyBox->GetTexture())
			mRenderSys.SetTexture(E_TEXTURE_ENV, skyBox->GetTexture());
	}
	else if (lightMode == E_PASS_POSTPROCESS) {
		if (camera.GetPostProcessInput()) 
			mRenderSys.SetTexture(E_TEXTURE_MAIN, camera.GetPostProcessInput()->GetAttachColorTexture(0));
	}
	else {

	}

	if (!lightsOrder.empty()) {
		BlendState originBlend = mRenderSys.GetBlendFunc();
		
		bool firstLight = true;
		for (size_t i = 0; i < lightsOrder.size(); ++i) {
			if (lightsOrder[i]->GetCameraMask() & camera.GetCameraMask()) {
				cbGlobalParam globalParam;
				cbPerLight lightParam;
				std::tie(globalParam, lightParam) = MakeAutoParam(camera, lightMode == E_PASS_SHADOWCASTER, *lightsOrder[0]);

				if (firstLight) {
					firstLight = false;
					mRenderSys.SetBlendFunc(BlendState::MakeAlphaNonPremultiplied());
					RenderLight(opQueue, lightMode, camera.GetCameraMask(), lightParam, globalParam);
				}
				else {
					mRenderSys.SetBlendFunc(BlendState::MakeAdditive());
					auto lightModeEx = (lightMode != E_PASS_FORWARDBASE) ? lightMode : E_PASS_FORWARDADD;
					RenderLight(opQueue, lightModeEx, camera.GetCameraMask(), lightParam, globalParam);
				}
			}
		}
		mRenderSys.SetBlendFunc(originBlend);
	}

	if (lightMode == E_PASS_SHADOWCASTER) {
	#if !defined DEBUG_SHADOW_CASTER
		_PopFrameBuffer();
	#endif
		mRenderSys.SetDepthState(orgDS);
		mRenderSys.SetBlendFunc(orgBS);

		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
		mRenderSys.SetTexture(E_TEXTURE_ENV, nullptr);
	}
	else if (lightMode == E_PASS_FORWARDBASE) {
		mRenderSys.SetTexture(E_TEXTURE_DEPTH_MAP, nullptr);
	}
	else {

	}
}

void RenderPipeline::RenderCamera(const RenderOperationQueue& opQueue, const Camera& camera, 
	const std::vector<ILightPtr>& lights)
{
	RenderOpQueue(opQueue, camera, lights, E_PASS_SHADOWCASTER);
#if !defined DEBUG_SHADOW_CASTER
	RenderOpQueue(opQueue, camera, lights, E_PASS_FORWARDBASE);
	//RenderOpQueue(opQueue, camera, lights, E_PASS_POSTPROCESS);
#endif
}

void RenderPipeline::Render(const RenderOperationQueue& opQueue, SceneManager& scene)
{
	for (auto& camera : scene.GetCameras()) 
	{
		//camera's output as framebuffer
		if (camera->GetOutput()) {
			_PushFrameBuffer(camera->GetOutput());
			mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f(0,0,0,0), 1.0, 0);
		}

		//camera's post_process_input as framebuffer
		if (!camera->GetPostProcessEffects().empty() && camera->GetPostProcessInput()) {
			_PushFrameBuffer(camera->GetPostProcessInput());
			mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f(0,0,0,0), 1.0, 0);
		}

		//render camera's skybox
		if (camera->GetSkyBox()) {
			RenderOperationQueue opQue;
			camera->GetSkyBox()->GenRenderOperation(opQue);
			RenderOpQueue(opQue, *camera, scene.GetLights(), E_PASS_FORWARDBASE);
		}

		//render camera itself
		RenderCamera(opQueue, *camera, scene.GetLights());

		//render camera's postprocess
		if (!camera->GetPostProcessEffects().empty() && camera->GetPostProcessInput()) {
			DepthState orgState = mRenderSys.GetDepthState();
			mRenderSys.SetDepthState(DepthState::MakeFor3D(false));

			RenderOperationQueue opQue;
			auto& postProcessEffects = camera->GetPostProcessEffects();
			for (size_t i = 0; i < postProcessEffects.size(); ++i)
				postProcessEffects[i]->GenRenderOperation(opQue);
			RenderOpQueue(opQue, *camera, scene.GetLights(), E_PASS_POSTPROCESS);

			mRenderSys.SetDepthState(orgState);
		}

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