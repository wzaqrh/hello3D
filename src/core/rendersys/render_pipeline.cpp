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

struct cbPerFrameBuilder {
	cbPerFrameBuilder& SetCamera(const scene::Camera& camera) {
		mCBuffer.View = camera.GetView();
		mCBuffer.Projection = camera.GetProjection();

		mCBuffer.ViewInv = mCBuffer.View.inverse();
		mCBuffer.ProjectionInv = mCBuffer.Projection.inverse();
		return *this;
	}
	void SetBackFrameBufferSize(Eigen::Vector2i backBufferSize) {
		mBackBufferSize = backBufferSize;
	}
	cbPerFrameBuilder& SetShadowMap(const IFrameBuffer& shadowMap) {
		auto smSize = shadowMap.GetSize();
		mCBuffer.ShadowMapSize = Eigen::Vector4f(smSize.x(), smSize.y(), 1.0 / smSize.x(), 1.0 / smSize.y());
		return *this;
	}
	void _SetFrameBuffer(IFrameBufferPtr frameBuffer) {
		auto fbSize = IF_AND_OR(frameBuffer, frameBuffer->GetSize(), mBackBufferSize);
		mCBuffer.FrameBufferSize = Eigen::Vector4f(fbSize.x(), fbSize.y(), 1.0f / fbSize.x(), 1.0f / fbSize.y());
	}
	cbPerFrameBuilder& SetFrameBuffer(IFrameBufferPtr frameBuffer) {
		_SetFrameBuffer(frameBuffer);
		return *this;
	}
	cbPerFrameBuilder& SetLight(const scene::Light& light) {
		mCBuffer.LightView = light.GetView();
		mCBuffer.LightProjection = light.GetRecvShadowProj();
		return *this;
	}
	cbPerFrame& Build() {
		return mCBuffer;
	}
	cbPerFrame& operator*() {
		return mCBuffer;
	}
private:
	Eigen::Vector2i mBackBufferSize;
	cbPerFrame mCBuffer;
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
		, mFbBank(Pipe.mTempFbs)
		, mShadowMap(Pipe.mShadowMap)
		, mGBuffer(Pipe.mGBuffer)
		, mGBufferSprite(Pipe.mGBufferSprite)
		, Ops(ops)
		, Camera(camera)
		, CameraMask(camera.GetCullingMask())
		, Lights(lights)
	{
		mPerFrame.SetCamera(camera);
		mPerFrame.SetBackFrameBufferSize(Pipe.mRenderSys.WinSize());
		mPerFrame.SetShadowMap(*mShadowMap);

		for (auto& light : lights) {
			if (light->GetCameraMask() & CameraMask) {
				Lights.push_back(light);

				if (mFirstLight == nullptr)
					mFirstLight = light;

				if (mMainLight == nullptr && light->DidCastShadow()) 
					mMainLight = light;
			}
		}
	}
public:
	#define MakePerLight(light) ((light).MakeCbLight())
	//LIGHTMODE_SHADOW_CASTER
	void RenderCastShadow()
	{
		if (mMainLight == nullptr) return;
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		auto shadow_clr_color = IF_AND_OR(mCfg.IsShadowVSM(), Eigen::Vector4f(1e4, 1e8, 0, 0), Eigen::Vector4f::Zero());
		auto fb_shadow_map = mStatesBlock.LockFrameBuffer(mShadowMap, shadow_clr_color, 1.0, 0);
		fb_shadow_map.SetCallback(std::bind(&cbPerFrameBuilder::_SetFrameBuffer, mPerFrame, std::placeholders::_1));

		depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
		blend_state(BlendState::MakeDisable());

		RenderLight(*mPerFrame.SetLight(*mMainLight), &MakePerLight(*mMainLight), LIGHTMODE_SHADOW_CASTER, true);

		if (mCfg.IsShadowVSM())
		{
			mGrabDic["_ShadowMap"] = mShadowMap;
			depth_state(DepthState::MakeFor3D(false));

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

			RenderLight(*mPerFrame.SetLight(*mMainLight), &MakePerLight(*mMainLight), LIGHTMODE_SHADOW_CASTER "+1", true, &ops);

			mRenderSys.GenerateMips(mShadowMap->GetAttachColorTexture(0));
		}
	}
	//LIGHTMODE_FORWARD_BASE, LIGHTMODE_FORWARD_ADD
	void RenderForward()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		auto tex_shadow_map = mStatesBlock.LockTexture(kPipeTextureShadowMap, IF_AND_OR(mCfg.IsShadowVSM(), mShadowMap->GetAttachColorTexture(0), mShadowMap->GetAttachZStencilTexture()));
		
		auto tex_env_diffuse = mStatesBlock.LockTexture(kPipeTextureDiffuseEnv, NULLABLE(Camera.GetSkyBox(), GetDiffuseEnvMap()));
		auto tex_env_spec = mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(Camera.GetSkyBox(), GetTexture()));
		auto tex_env_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(Camera.GetSkyBox(), GetLutMap()));

		for (auto& light : Lights)
		{
			if (mFirstLight == light)
			{
				depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));
				blend_state(BlendState::MakeDisable());

				RenderLight(*mPerFrame.SetLight(*light), &MakePerLight(*light), LIGHTMODE_FORWARD_BASE);
			}
			else {
				depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskZero));
				blend_state(BlendState::MakeAdditive());

				RenderLight(*mPerFrame.SetLight(*light), &MakePerLight(*light), LIGHTMODE_FORWARD_ADD);
			}
		}
	}
	//LIGHTMODE_PREPASS_BASE, LIGHTMODE_PREPASS_FINAL
	void RenderPrepass()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		//LIGHTMODE_PREPASS_BASE
		{
			blend_state(BlendState::MakeDisable());
			depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));

			auto fb_gbuffer = mStatesBlock.LockFrameBuffer(mGBuffer, Eigen::Vector4f::Zero(), 1.0, 0);
			fb_gbuffer.SetCallback(std::bind(&cbPerFrameBuilder::_SetFrameBuffer, mPerFrame, std::placeholders::_1));

			RenderLight(*mPerFrame.SetLight(*mMainLight), &MakePerLight(*mMainLight), LIGHTMODE_PREPASS_BASE);
		}

		//LIGHTMODE_PREPASS_FINAL
		for (auto& light : Lights)
		{
			blend_state(IF_AND_OR(light == mFirstLight, BlendState::MakeAlphaNonPremultiplied(), BlendState::MakeAdditive()));
			depth_state(DepthState::Make(kCompareLess, kDepthWriteMaskAll));

			auto tex_gdepth = mStatesBlock.LockTexture(kPipeTextureGDepth, mGBuffer->GetAttachZStencilTexture());
			auto tex_shadow_map = mStatesBlock.LockTexture(kPipeTextureShadowMap, IF_AND_OR(mCfg.IsShadowVSM(), mShadowMap->GetAttachColorTexture(0), mShadowMap->GetAttachZStencilTexture()));

			auto tex_env_spec = mStatesBlock.LockTexture(kPipeTextureSpecEnv, NULLABLE(Camera.GetSkyBox(), GetTexture()));
			auto tex_env_diffuse = mStatesBlock.LockTexture(kPipeTextureDiffuseEnv, NULLABLE(Camera.GetSkyBox(), GetDiffuseEnvMap()));
			auto tex_env_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(Camera.GetSkyBox(), GetLutMap()));

			auto tex_gpos = mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
			auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));
			auto tex_galbedo = mStatesBlock.LockTexture(kPipeTextureGBufferAlbedo, mGBuffer->GetAttachColorTexture(2));
			auto tex_gemissive = mStatesBlock.LockTexture(kPipeTextureGBufferEmissive, mGBuffer->GetAttachColorTexture(3));

			RenderOperationQueue ops;
			mGBufferSprite->GenRenderOperation(ops);
			RenderLight(*mPerFrame.SetLight(*light), &MakePerLight(*light), LIGHTMODE_PREPASS_FINAL, false, &ops);
		}
	}
	void RenderSkybox()
	{
		auto skybox = Camera.GetSkyBox();
		if (skybox == nullptr) return;
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		depth_state(DepthState::Make(kCompareLessEqual, kDepthWriteMaskZero));
		blend_state(BlendState::MakeDisable());

		RenderOperationQueue ops;
		skybox->GenRenderOperation(ops);
		for (const auto& op : ops) {
			auto shader = op.Material->GetShader();
			if ((op.CameraMask & CameraMask) && shader->IsLoaded()) {
				(*mPerFrame).World = op.WorldTransform;

				op.Material.WriteToCb(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(*mPerFrame));

				op.Material.FlushGpuParameters(mRenderSys);
				RenderOp(op, LIGHTMODE_FORWARD_BASE);
			}
		}
	}
	//LIGHTMODE_OVERLAY
	void RenderOverlay()
	{
		auto blend_state = mStatesBlock.LockBlend(BlendState::MakeAlphaNonPremultiplied());
		auto depth_state = mStatesBlock.LockDepth(DepthState::MakeFor3D(false));

		RenderLight(*mPerFrame, nullptr, LIGHTMODE_OVERLAY);
	}
	//LIGHTMODE_POSTPROCESS
	void RenderPostProcess(const IFrameBufferPtr& fbInput)
	{
		auto blend_state = mStatesBlock.LockBlend();
		auto depth_state = mStatesBlock.LockDepth();

		blend_state(BlendState::MakeDisable());
		depth_state(DepthState::MakeFor3D(false));

		auto scene_image = Camera.GetPostProcessInput();
		auto& effects = Camera.GetPostProcessEffects();
		std::vector<IFrameBufferPtr> tempOutputs(effects.size());
		for (size_t i = 0; i < tempOutputs.size() - 1; ++i)
			tempOutputs[i] = mFbBank->Borrow();
		tempOutputs.back() = Camera.GetOutput();

		for (size_t i = 0; i < effects.size(); ++i) {
			auto fb_temp_output = mStatesBlock.LockFrameBuffer(tempOutputs[i]);
			fb_temp_output.SetCallback(std::bind(&cbPerFrameBuilder::_SetFrameBuffer, mPerFrame, std::placeholders::_1));
			
			auto tex_scene_img = mStatesBlock.LockTexture(kPipeTextureSceneImage, IF_AND_OR(i == 0, scene_image->GetAttachColorTexture(0), tempOutputs[i - 1]->GetAttachColorTexture(0)));
			auto tex_shadow_map = mStatesBlock.LockTexture(kPipeTextureGDepth, mGBuffer->GetAttachZStencilTexture());
			
			auto tex_gpos = mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
			auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));

			RenderOperationQueue opQue;
			effects[i]->GenRenderOperation(opQue);
			RenderLight(*mPerFrame, nullptr, LIGHTMODE_POSTPROCESS);
		}
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
					passFb = mFbBank->Borrow(passOut.Formats, passOut.Size);
					mTempGrabDic[passOut.Name] = passFb;
				}
				mStatesBlock.FrameBuffer.Push(passFb);
				mPerFrame.SetFrameBuffer(mStatesBlock.CurrentFrameBuffer());
			}
			const auto& passIn = pass->GetGrabIn();
			if (passIn) {
				IFrameBufferPtr passFb = mTempGrabDic[passIn.Name];
				BOOST_ASSERT(passFb);
				if (passFb) mStatesBlock.Textures(passIn.TextureSlot, IF_AND_OR(passIn.AttachIndex >= 0, passFb->GetAttachColorTexture(passIn.AttachIndex), passFb->GetAttachZStencilTexture()));
			}

			RenderPass(pass, op);

			if (passIn) {
				mStatesBlock.Textures(passIn.TextureSlot, nullptr);
			}
			if (passOut) {
				mStatesBlock.FrameBuffer.Pop();
				mPerFrame.SetFrameBuffer(mStatesBlock.CurrentFrameBuffer());
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
	FrameBufferBankPtr mFbBank;
	IFrameBufferPtr mShadowMap, mGBuffer;
	rend::SpritePtr mGBufferSprite;
private:
	const RenderOperationQueue& Ops;
	const scene::Camera& Camera;
	const unsigned CameraMask;
	std::vector<scene::LightPtr> Lights;
private:
	std::map<std::string, IFrameBufferPtr> mTempGrabDic, mGrabDic;
	scene::LightPtr mMainLight, mFirstLight;
	cbPerFrameBuilder mPerFrame;
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

	if (camera.GetPostProcessInput())
		mStatesBlock.FrameBuffer.Push(camera.GetPostProcessInput());

	mRenderSys.ClearFrameBuffer(mStatesBlock.CurrentFrameBuffer(), Eigen::Vector4f::Zero(), 1.0, 0);

	CameraRender render(*this, ops, camera, lights);
	render.RenderCastShadow();
	render.RenderForward();
	render.RenderSkybox();
	render.RenderOverlay();

	if (camera.GetPostProcessInput()) {
		mStatesBlock.FrameBuffer.Pop();

		render.RenderPostProcess(camera.GetPostProcessInput());
	}
}

void RenderPipeline::RenderCameraDeffered(const RenderOperationQueue& ops, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights)
{
	if (lights.empty()) return;

	if (camera.GetPostProcessInput())
		mStatesBlock.FrameBuffer.Push(camera.GetPostProcessInput());

	mRenderSys.ClearFrameBuffer(mStatesBlock.CurrentFrameBuffer(), Eigen::Vector4f::Zero(), 1.0, 0);

	CameraRender render(*this, ops, camera, lights);
	render.RenderCastShadow();
	render.RenderPrepass();
	render.RenderSkybox();
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