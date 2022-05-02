#include <boost/format.hpp>
#include "core/rendersys/render_pipeline.h"
#include "core/rendersys/render_states_block.h"
#include "core/rendersys/frame_buffer_bank.h"
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

enum PipeLineTextureSlot 
{
	kPipeTextureSceneImage = 6,
	kPipeTextureGDepth = 7,
	kPipeTextureShadowMap = 8,
	kPipeTextureDiffuseEnv = 9,
	kPipeTextureSpecEnv = 10,
	kPipeTextureLUT = 11,
	kPipeTextureGBufferPos = 12,
	kPipeTextureGBufferNormal = 13,
	kPipeTextureGBufferAlbedo = 14,
	kPipeTextureGBufferEmissive = 15
};

class CameraRender 
{
public:
	CameraRender(CameraRender& other, const RenderOperationQueue& ops)
		:CameraRender(other.Pipe, ops, other.Camera, other.Lights)
	{}
	CameraRender(RenderPipeline& pipe,
		const RenderOperationQueue& ops,
		const scene::Camera& camera,
		const std::vector<scene::LightPtr>& lights)
		: Pipe(pipe)
		, mCfg(Pipe.mCfg)
		, mRenderSys(Pipe.mRenderSys)
		, mStatesBlock(Pipe.mStatesBlock)
		, mTempFbs(Pipe.mTempFbs)
		, mShadowMap(Pipe.mShadowMap)
		, mGBuffer(Pipe.mGBuffer)
		, mGBufferSprite(Pipe.mGBufferSprite)
		, Ops(ops)
		, Camera(camera)
		, CameraMask(camera.GetCullingMask())
		, Lights(lights)
	{}
	//LIGHTMODE_SHADOW_CASTER
	void RenderCastShadow()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();
		scene::LightPtr firstLight = nullptr;
		{
			depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
			blend_state(BlendState::MakeDisable());
			auto fb_shadow_map = mStatesBlock.LockFrameBuffer(mShadowMap, 
				IF_AND_OR(mCfg.IsShadowVSM(), Eigen::Vector4f(1e4, 1e8, 0, 0), Eigen::Vector4f::Zero()), 1.0, 0);
			for (auto& light : Lights) {
				if ((light->GetCameraMask() & CameraMask) && (light->DidCastShadow())) {
					cbPerFrame perFrame = MakeCastShadowPerFrame(Camera, mRenderSys.WinSize(), *light);
					RenderLight(perFrame, &MakePerLight(*light), LIGHTMODE_SHADOW_CASTER, true);
					firstLight = light;
					break;
				}
			}
		}
		if (mCfg.IsShadowVSM() && firstLight)
		{
			mGrabDic["__shadowmap"] = mShadowMap;
			depth_state(DepthState::MakeFor3D(false));
			cbPerFrame perFrame = MakeReceiveShadowPerFrame(Camera, mRenderSys.WinSize(), *firstLight, mShadowMap);

			res::MaterialInstance mat;
			for (auto& op : Ops) {
				if (op.CastShadow) {
					mat = op.Material;
					break;
				}
			}
			BOOST_ASSERT(mat);

			RenderOperationQueue ops;
			mGBufferSprite->GenRenderOperation(ops);
			for (auto& op : ops)
				op.Material = mat;

			RenderLight(perFrame, &MakePerLight(*firstLight), LIGHTMODE_SHADOW_CASTER "+1", true, &ops);

			mRenderSys.GenerateMips(mShadowMap->GetAttachColorTexture(0));
		}
	}
	void RenderSkybox(const rend::SkyBoxPtr& skybox, BlendStateBlock::Lock& blend_state, DepthStateBlock::Lock& depth_state)
	{
		depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskZero));
		blend_state(BlendState::MakeDisable());
		RenderOperationQueue ops;
		skybox->GenRenderOperation(ops);

		cbPerFrame perFrame = MakePerFrame(Camera, mRenderSys.WinSize());
		const cbPerLight* lightParam = nullptr;
		for (const auto& op : ops) {
			auto shader = op.Material->GetShader();
			if ((op.CameraMask & CameraMask) && shader->IsLoaded()) {
				perFrame.World = op.WorldTransform;

				op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(perFrame));
				if (lightParam) op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*lightParam));

				op.Material.FlushGpuParameters(mRenderSys);
				RenderOp(op, LIGHTMODE_FORWARD_BASE);
			}
		}
	}
	//LIGHTMODE_FORWARD_BASE, LIGHTMODE_FORWARD_ADD
	void RenderForward()
	{
		auto fb_post_process_input = mStatesBlock.LockFrameBuffer(IF_OR(Camera.GetPostProcessInput(), nullptr), Eigen::Vector4f::Zero(), 1.0, 0);
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		auto tex_shadow = mStatesBlock.LockTexture(kPipeTextureShadowMap, 
			IF_AND_OR(mCfg.IsShadowVSM(), mShadowMap->GetAttachColorTexture(0), mShadowMap->GetAttachZStencilTexture()));
		auto tex_diffuse_env = mStatesBlock.LockTexture(kPipeTextureDiffuseEnv, NULLABLE(Camera.GetSkyBox(), GetDiffuseEnvMap()));
		auto tex_spec_env = mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(Camera.GetSkyBox(), GetTexture()));
		auto tex_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(Camera.GetSkyBox(), GetLutMap()));

		scene::LightPtr firstLight = nullptr;
		cbPerFrame perFrame;
		for (auto& light : Lights)
		{
			if (light->GetCameraMask() & Camera.GetCullingMask())
			{
				if (firstLight == nullptr)
				{
					firstLight = light;

					depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
					blend_state(BlendState::MakeAlphaNonPremultiplied());
					perFrame = MakeReceiveShadowPerFrame(Camera, mRenderSys.WinSize(), *firstLight, mShadowMap);
					RenderLight(perFrame, &MakePerLight(*light), LIGHTMODE_FORWARD_BASE);

					if (auto skybox = Camera.GetSkyBox()) 
						RenderSkybox(skybox, blend_state, depth_state);
				}
				else {
					depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskZero));
					blend_state(BlendState::MakeAdditive());
					RenderLight(perFrame, &MakePerLight(*light), LIGHTMODE_FORWARD_ADD);
				}
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights
	}
	//LIGHTMODE_PREPASS_BASE
	void RenderPrepass()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();
		mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);

		scene::LightPtr firstLight = nullptr;
		cbPerFrame perFrame;
		for (auto& light : Lights)
		{
			if (light->GetCameraMask() & Camera.GetCullingMask())
			{
				if (firstLight == nullptr)
				{
					firstLight = light;

					auto fb_gbuffer = mStatesBlock.LockFrameBuffer(mGBuffer);
					mRenderSys.ClearFrameBuffer(nullptr, Eigen::Vector4f::Zero(), 1.0, 0);
					blend_state(BlendState::MakeDisable());
					depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));

					perFrame = MakeReceiveShadowPerFrame(Camera, mRenderSys.WinSize(), *firstLight, mGBuffer);
					RenderLight(perFrame, &MakePerLight(*light), LIGHTMODE_PREPASS_BASE);

					blend_state(BlendState::MakeAlphaNonPremultiplied());
				}
				else {
					blend_state(BlendState::MakeAdditive());
				}
				depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));

				//LIGHTMODE_PREPASS_FINAL
				auto tex_gdepth = mStatesBlock.LockTexture(kPipeTextureGDepth, mGBuffer->GetAttachZStencilTexture());
				auto tex_shadow = mStatesBlock.LockTexture(kPipeTextureShadowMap, 
					IF_AND_OR(mCfg.IsShadowVSM(), mShadowMap->GetAttachColorTexture(0), mShadowMap->GetAttachZStencilTexture()));

				auto tex_spec_env = mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(Camera.GetSkyBox(), GetTexture()));
				auto tex_diffuse_env = mStatesBlock.LockTexture(kPipeTextureDiffuseEnv, NULLABLE(Camera.GetSkyBox(), GetDiffuseEnvMap()));
				auto tex_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(Camera.GetSkyBox(), GetLutMap()));

				auto tex_gpos = mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
				auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));
				auto tex_galbedo = mStatesBlock.LockTexture(kPipeTextureGBufferAlbedo, mGBuffer->GetAttachColorTexture(2));
				auto tex_gemissive = mStatesBlock.LockTexture(kPipeTextureGBufferEmissive, mGBuffer->GetAttachColorTexture(3));
				RenderOperationQueue ops;
				mGBufferSprite->GenRenderOperation(ops);
				RenderLight(perFrame, &MakePerLight(*light), LIGHTMODE_PREPASS_FINAL, false, &ops);
			}//if light.GetCameraMask & camera.GetCameraMask
		}//for lights

		if (auto skybox = Camera.GetSkyBox())
			RenderSkybox(skybox, blend_state, depth_state);
	}
	//LIGHTMODE_OVERLAY
	void RenderOverlay()
	{
		auto blend_state = mStatesBlock.LockBlend(BlendState::MakeAlphaNonPremultiplied());
		auto depth_state = mStatesBlock.LockDepth(DepthState::MakeFor3D(false));

		cbPerFrame perFrame = MakePerFrame(Camera, mRenderSys.WinSize());
		RenderLight(perFrame, nullptr, LIGHTMODE_OVERLAY);
	}
	//LIGHTMODE_POSTPROCESS
	void RenderPostProcess(const IFrameBufferPtr& fbInput)
	{
		auto blend_state = mStatesBlock.LockBlend();
		auto depth_state = mStatesBlock.LockDepth();
		blend_state(BlendState::MakeDisable());
		depth_state(DepthState::MakeFor3D(false));

		auto postProcessInput = Camera.GetPostProcessInput();
		auto& postProcessEffects = Camera.GetPostProcessEffects();
		std::vector<IFrameBufferPtr> tempOutputs(postProcessEffects.size());
		for (size_t i = 0; i < tempOutputs.size() - 1; ++i)
			tempOutputs[i] = mTempFbs->Borrow();
		tempOutputs.back() = Camera.GetOutput();

		for (size_t i = 0; i < postProcessEffects.size(); ++i) {
			auto curfb = mStatesBlock.LockFrameBuffer(tempOutputs[i]);
			auto tex_sceneimg = mStatesBlock.LockTexture(kPipeTextureSceneImage, IF_AND_OR(i == 0, postProcessInput->GetAttachColorTexture(0), tempOutputs[i - 1]->GetAttachColorTexture(0)));
			auto tex_shadow = mStatesBlock.LockTexture(kPipeTextureGDepth, mGBuffer->GetAttachZStencilTexture());
			auto tex_gpos = mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
			auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));

			RenderOperationQueue opQue;
			postProcessEffects[i]->GenRenderOperation(opQue);
			cbPerFrame perFrame = MakePerFrame(Camera, mRenderSys.WinSize());
			RenderLight(perFrame, nullptr, LIGHTMODE_POSTPROCESS);
		}
	}
public:
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
	static cbPerFrame MakePerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize, IFrameBufferPtr shadowMap = nullptr)
	{
		cbPerFrame perFrameParam;
		perFrameParam.View = camera.GetView();
		perFrameParam.Projection = camera.GetProjection();
		if (shadowMap) perFrameParam.ShadowMapSize = Eigen::Vector4f(shadowMap->GetWidth(), shadowMap->GetHeight(), 1.0 / shadowMap->GetWidth(), 1.0 / shadowMap->GetHeight());
		return MakePerFrame(perFrameParam, camera.GetTransform()->GetLocalPosition(), fbSize);
	}
	static cbPerFrame MakeReceiveShadowPerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize, const scene::Light& light, IFrameBufferPtr shadowMap)
	{
		cbPerFrame perFrameParam = MakePerFrame(camera, fbSize);
		if (shadowMap) {
			perFrameParam.LightView = light.GetView();
			perFrameParam.LightProjection = light.GetRecvShadowProj();
			perFrameParam.ShadowMapSize = Eigen::Vector4f(shadowMap->GetWidth(), shadowMap->GetHeight(), 1.0 / shadowMap->GetWidth(), 1.0 / shadowMap->GetHeight());
		}
		return perFrameParam;
	}
	static cbPerFrame MakeCastShadowPerFrame(const scene::Camera& camera, const Eigen::Vector2i& fbSize, const scene::Light& light)
	{
		cbPerFrame perFrameParam;
		perFrameParam.View = light.GetView();
		perFrameParam.Projection = light.GetCastShadowProj();
		return MakePerFrame(perFrameParam, camera.GetTransform()->GetLocalPosition(), fbSize);
	}
private:
	void RenderLight(cbPerFrame& globalParam, const cbPerLight* lightParam, const std::string& lightMode, bool castShadow = false, const RenderOperationQueue* ops = nullptr)
	{
		if (ops == nullptr) ops = &Ops;

		for (const auto& op : *ops) {
			auto shader = op.Material->GetShader();
			if (castShadow && (!op.CastShadow))
				continue;
			if ((op.CameraMask & CameraMask) && shader->IsLoaded()) {
				globalParam.World = op.WorldTransform;

				op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(globalParam));
				if (lightParam) op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*lightParam));

				op.Material.FlushGpuParameters(mRenderSys);
				RenderOp(op, lightMode);
			}
		}
	}
	void RenderOp(const RenderOperation& op, const std::string& lightMode)
	{
		mTempGrabDic = mGrabDic;
		res::TechniquePtr tech = op.Material->GetShader()->CurTech();
		std::vector<res::PassPtr> passes = tech->GetPassesByLightMode(lightMode);
		for (auto& pass : passes) {
			const auto& passOut = pass->GetGrabOut();
			if (passOut) {
				IFrameBufferPtr passFb = mTempGrabDic[passOut.Name];
				if (passFb == nullptr) {
					passFb = mTempFbs->Borrow(passOut.Formats);
					mTempGrabDic[passOut.Name] = passFb;
				}
				mStatesBlock.FrameBuffer.Push(passFb);
			}
			const auto& passIn = pass->GetGrabIn();
			if (passIn) {
				IFrameBufferPtr passFb = mTempGrabDic[passIn.Name];
				if (passFb) mStatesBlock.Textures(passIn.TextureSlot, 
					IF_AND_OR(passIn.AttachIndex >= 0, passFb->GetAttachColorTexture(passIn.AttachIndex), passFb->GetAttachZStencilTexture()));
			}

			RenderPass(pass, op);

			if (passOut) {
				mStatesBlock.FrameBuffer.Pop();
			}
			if (passIn) {
				mStatesBlock.Textures(passIn.TextureSlot, nullptr);
			}
		}
	}
	void RenderPass(const res::PassPtr& pass, const RenderOperation& op)
	{
		//SetVertexLayout(pass->mInputLayout);
		mRenderSys.SetVertexBuffers(op.VertexBuffers);
		mRenderSys.SetIndexBuffer(op.IndexBuffer);
		mRenderSys.SetConstBuffers(op.Material.GetConstBuffers(), pass->GetProgram());

		const TextureVector& textures = op.Material->GetTextures();
		if (textures.Count() > 0) mStatesBlock.Textures(kTextureUserSlotFirst, &textures[0], textures.Count());
		else mStatesBlock.Textures(kTextureUserSlotFirst, nullptr, 0);

		mRenderSys.SetProgram(pass->GetProgram());
		mRenderSys.SetVertexLayout(pass->GetInputLayout());

		const auto& samplers = pass->GetSamplers();
		if (!samplers.empty()) mRenderSys.SetSamplers(0, &samplers[0], samplers.size());
		else mRenderSys.SetSamplers(0, nullptr, 0);

		if (op.IndexBuffer) mRenderSys.DrawIndexedPrimitive(op, pass->GetTopoLogy());
		else mRenderSys.DrawPrimitive(op, pass->GetTopoLogy());
	}
private:
	RenderPipeline& Pipe;
	const Configure& mCfg;
	RenderSystem& mRenderSys;
	RenderStatesBlock& mStatesBlock;
	FrameBufferBankPtr mTempFbs;
	IFrameBufferPtr mShadowMap, mGBuffer;
	rend::SpritePtr mGBufferSprite;
private:
	const RenderOperationQueue& Ops;
	const scene::Camera& Camera;
	unsigned CameraMask;
	const std::vector<scene::LightPtr>& Lights;
private:
	std::map<std::string, IFrameBufferPtr> mTempGrabDic, mGrabDic;
};

RenderPipeline::RenderPipeline(RenderSystem& renderSys, ResourceManager& resMng, const Configure& cfg)
	: mRenderSys(renderSys)
	, mCfg(cfg)
	, mStatesBlockPtr(CreateInstance<RenderStatesBlock>(renderSys))
	, mStatesBlock(*mStatesBlockPtr)
{
	Eigen::Vector3i fbSize = Eigen::Vector3i(resMng.WinWidth(), resMng.WinHeight(), 1);
	
	if (mCfg.IsShadowVSM()) mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, Eigen::Vector3i(fbSize.x(), fbSize.y(), 0), MakeResFormats(kFormatR32G32Float, kFormatD24UNormS8UInt));
	else mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, fbSize, MakeResFormats(kFormatR8G8B8A8UNorm, kFormatD24UNormS8UInt));
	DEBUG_SET_PRIV_DATA(mShadowMap, "render_pipeline.shadow_map");

	mGBuffer = resMng.CreateFrameBuffer(__LaunchSync__, fbSize, 
		MakeResFormats(kFormatR16G16B16A16UNorm,//Pos 
			kFormatR16G16B16A16UNorm,//Normal 
			kFormatR8G8B8A8UNorm,//Albedo 
			kFormatR8G8B8A8UNorm,//Emissive 
			kFormatD24UNormS8UInt));
	DEBUG_SET_PRIV_DATA(mGBuffer, "render_pipeline.gbuffer");//Pos, Normal, Albedo, Emissive

	mTempFbs = CreateInstance<FrameBufferBank>(resMng, fbSize, MakeResFormats(kFormatR8G8B8A8UNorm, kFormatD24UNormS8UInt));

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

void RenderPipeline::RenderCameraForward(const RenderOperationQueue& ops, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights)
{
	if (lights.empty()) return;

	CameraRender render(*this, ops, camera, lights);

	render.RenderCastShadow();
	render.RenderForward();
	render.RenderOverlay();
}

void RenderPipeline::RenderCameraDeffered(const RenderOperationQueue& ops, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights)
{
	if (lights.empty()) return;

	if (camera.GetPostProcessInput())
		mStatesBlock.FrameBuffer.Push(camera.GetPostProcessInput());

	CameraRender render(*this, ops, camera, lights);
	render.RenderCastShadow();
	render.RenderPrepass();
	render.RenderOverlay();
	if (camera.GetPostProcessInput()) {
		mStatesBlock.FrameBuffer.Pop();
		render.RenderPostProcess(camera.GetPostProcessInput());
	}
}

void RenderPipeline::Render(const RenderOperationQueue& opQueue, const std::vector<scene::CameraPtr>& cameras, const std::vector<scene::LightPtr>& lights)
{
	for (auto& camera : cameras) 
	{
		auto fb_camera_output = mStatesBlock.LockFrameBuffer(IF_OR(camera->GetOutput(), nullptr));
		if (camera->GetRenderingPath() == kRenderPathForward) RenderCameraForward(opQueue, *camera, lights);
		else RenderCameraDeffered(opQueue, *camera, lights);
	}
}

bool RenderPipeline::BeginFrame()
{
	return mRenderSys.BeginScene();
}
void RenderPipeline::EndFrame()
{
	mRenderSys.EndScene();
	mTempFbs->ReturnAll();
}

}