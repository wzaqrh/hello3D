#include <boost/format.hpp>
#include "core/mir_config_macros.h"
#include "core/base/macros.h"
#include "core/base/debug.h"
#include "core/base/attribute_struct.h"
#include "core/rendersys/render_pipeline.h"
#include "core/rendersys/render_states_block.h"
#include "core/rendersys/frame_buffer_bank.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_name.h"
#include "core/resource/material_factory.h"
#include "core/renderable/sprite.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/scene/scene_manager.h"
#include "core/scene/light.h"
#include "core/scene/camera.h"
#include "test/unit_test/unit_test.h"

namespace mir {

enum PipeLineTextureSlot 
{
	kPipeTextureGDepth = 0,
	kPipeTextureGBufferPos = 1,
	kPipeTextureGBufferNormal = 2,
	kPipeTextureGBufferAlbedo = 3,
	kPipeTextureGBufferEmissive = 4,
	kPipeTextureGBufferSheen = 5,
	kPipeTextureGBufferClearCoat = 6,

	kPipeTextureSceneImage = 8,
	kPipeTextureLightMap = 8,

	kPipeTextureShadowMap = 11,
	kPipeTextureEnvSheen = 12,
	kPipeTextureEnvDiffuse = 13,
	kPipeTextureEnvSpec = 14,
	kPipeTextureLUT = 15,
};

#define kDepthFormat kFormatD24UNormS8UInt//kFormatD24UNormS8UInt

struct cbPerFrameBuilder 
{
	cbPerFrameBuilder& SetSkybox(const rend::SkyBoxPtr& skybox) {
		auto shc = IF_AND_OR(skybox, skybox->GetSphericalHarmonicsConstants(), rend::SphericalHarmonicsConstants());
		mCBuffer.SHC0C1 = shc.C0C1;
		mCBuffer.SHC2 = shc.C2;
		mCBuffer.SHC2_2 = shc.C2_2;
		mCBuffer.EnvSpecColorMip.w() = NULLABLE_MEM(skybox->GetTexture(), GetMipmapCount(), 1);
		mCBuffer.EnvSheenColorMip.w() = NULLABLE_MEM(skybox->GetSheenMap(), GetMipmapCount(), 1);
		return *this;
	}
	cbPerFrameBuilder& SetCamera(const scene::Camera& camera) {
		mReversedZ = camera.IsReverseZ();

		mCBuffer.View = camera.GetView();
		mCBuffer.Projection = camera.GetProjection();

		mCBuffer.ViewInv = mCBuffer.View.inverse();
		mCBuffer.ProjectionInv = mCBuffer.Projection.inverse();

		mCBuffer.CameraPositionExposure.head<3>() = camera.GetTransform()->GetPosition();
		mCBuffer.CameraPositionExposure.w() = 1.0f;
		return SetSkybox(camera.GetSkyBox());
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
	cbPerFrameBuilder& SetLight(const scene::LightPtr& light) {
		if (light) {
			mCBuffer.LightView = light->GetView();
			mCBuffer.LightProjection = light->GetRecvShadowProj();
		}
		else {
			mCBuffer.LightProjection = mCBuffer.LightView = Eigen::Matrix4f::Identity();
		}
		return *this;
	}
	cbPerFrameBuilder& SetLightMapSize(const Eigen::Vector2i& size, int mipCount) {
		mCBuffer.LightMapSizeMip = Eigen::Vector4f(size.x(), size.y(), mipCount, 0);
		return *this;
	}
	float GetZFar() const { return IF_AND_OR(mReversedZ, 0.0, 1.0); }
	float GetZNear() const { return IF_AND_OR(mReversedZ, 1.0, 0.0); } 
	CompareFunc GetZFunc(CompareFunc func) const { return IF_AND_OR(mReversedZ, GetReverseZCompareFunc(func), func); }
	cbPerFrame& Build() {
		return mCBuffer;
	}
	cbPerFrame& operator*() {
		return mCBuffer;
	}
private:
	Eigen::Vector2i mBackBufferSize;
	bool mReversedZ = false;
	cbPerFrame mCBuffer;
};

class CameraRender 
{
public:
	CameraRender(CameraRender& other, const RenderableCollection& rends)
		:CameraRender(other.Pipe, rends, other.Camera, other.Lights)
	{}
	CameraRender(RenderPipeline& pipe,
		const RenderableCollection& rends,
		const scene::Camera& camera,
		const std::vector<scene::LightPtr>& lights)
		: Pipe(pipe)
		, mCfg(Pipe.mCfg)
		, mRenderSys(Pipe.mRenderSys)
		, mStatesBlock(Pipe.mStatesBlock)
		, mFbBank(Pipe.mFbsBank)
		, mShadowMap(Pipe.mShadowMap)
		, mGBuffer(Pipe.mGBuffer)
		, mGBufferSprite(Pipe.mGBufferSprite)
		, Rends(rends)
		, Camera(camera)
		, CameraMask(camera.GetCullingMask())
	{
		mPerFrame.SetCamera(camera);
		mPerFrame.SetBackFrameBufferSize(Pipe.mRenderSys.WinSize());
		mPerFrame.SetShadowMap(*mShadowMap);

		mGBufferSprite->SetPosition(Eigen::Vector3f(-1, -1, mPerFrame.GetZNear()));
		mGBufferSprite->SetSize(Eigen::Vector3f(2, 2, mPerFrame.GetZNear()));

		for (auto& light : lights) {
			if (light->GetCameraMask() & CameraMask) {
				Lights.push_back(light);

				if (mFirstLight == nullptr)
					mFirstLight = light;

				if (mMainLight == nullptr && light->DidCastShadow()) 
					mMainLight = light;
			}
		}
		if (Lights.empty()) Lights.push_back(nullptr);

		RenderOperationQueue ops;
		for (auto& r : Rends) {
			if (r->GetCameraMask() & CameraMask) {
				int position = ops.Count();
				r->GenRenderOperation(ops);
				for (int i = position; i < ops.Count(); ++i) {
					ops[i].CastShadow = r->IsCastShadow();
				}
			}
		}
		for (auto& op : ops) {
			auto renderType = op.Material->GetProperty().RenderType;
			BOOST_ASSERT(renderType > RENDER_TYPE_UNKOWN && renderType < RENDER_TYPE_MAX);
			mOpsByRT[renderType].AddOP(op);
		}
		for (const auto& op : mOpsByRT[RENDER_TYPE_GEOMETRY]) {
			if (op.CastShadow) {
				mCastShadowOps.AddOP(op);
			}
		}
		mGBufferSprite->GenRenderOperation(mDefferedOps);
	}
public:
	void RenderForwardPath()
	{
		RenderCastShadow();

		EnsureGeometrySkybox(RENDER_TYPE_TRANSPARENT, LIGHTMODE_FORWARD_BASE, [this]() {
			RenderForward();
			RenderSkybox();
		});
		RenderTransparent();

		RenderOverlay();
	}
	void RenderDefferedPath()
	{
		RenderCastShadow();

		EnsureGeometrySkybox(RENDER_TYPE_TRANSPARENT, LIGHTMODE_FORWARD_BASE, [this]() {
			RenderPrepassBase();
			RenderPrepassFinal();
			RenderSkybox();
		});
		RenderTransparent();

		RenderOverlay();
	}
public:
	void Clear() 
	{
		mRenderSys.ClearFrameBuffer(mStatesBlock.CurrentFrameBuffer(), Eigen::Vector4f::Zero(), mPerFrame.GetZFar(), 0);
	}
	void RenderCastShadow()
	{
		if (mMainLight == nullptr) return;

		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		auto shadow_clr_color = IF_AND_OR(mCfg.IsShadowVSM(), Eigen::Vector4f(1e4, 1e8, 0, 0), Eigen::Vector4f::Zero());
		auto fb_shadow_map = mStatesBlock.LockFrameBuffer(mShadowMap, shadow_clr_color, mPerFrame.GetZFar(), 0);
		fb_shadow_map.SetCallback(std::bind(&cbPerFrameBuilder::_SetFrameBuffer, mPerFrame, std::placeholders::_1));

		depth_state(DepthState::Make(mPerFrame.GetZFunc(kCompareLess), kDepthWriteMaskAll));
		blend_state(BlendState::MakeDisable());

		RenderLight(*mPerFrame.SetLight(mMainLight), MakePerLight(mMainLight), LIGHTMODE_SHADOW_CASTER, mCastShadowOps);

		if (mCfg.IsShadowVSM() && !mDefferedOps.IsEmpty())
		{
			mGrabDic["_ShadowMap"] = mShadowMap;
			depth_state(DepthState::MakeFor3D(false));

			RenderLight(*mPerFrame.SetLight(mMainLight), MakePerLight(mMainLight), LIGHTMODE_SHADOW_CASTER_POSTPROCESS, mDefferedOps);
			mRenderSys.GenerateMips(mShadowMap->GetAttachColorTexture(0));
		}
	}
	void RenderForward()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		auto attach_shadow_map = IF_AND_OR(mCfg.IsShadowVSM(), mShadowMap->GetAttachColorTexture(0), mShadowMap->GetAttachZStencilTexture());
		auto tex_shadow_map = mStatesBlock.LockTexture(kPipeTextureShadowMap, attach_shadow_map);
		
		auto tex_env_sheen = mStatesBlock.LockTexture(kPipeTextureEnvSheen, NULLABLE(Camera.GetSkyBox(), GetSheenMap()));
		auto tex_env_diffuse = mStatesBlock.LockTexture(kPipeTextureEnvDiffuse, NULLABLE(Camera.GetSkyBox(), GetDiffuseEnvMap()));
		auto tex_env_spec = mStatesBlock.LockTexture(kPipeTextureEnvSpec, NULLABLE(Camera.GetSkyBox(), GetTexture()));
		auto tex_env_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(Camera.GetSkyBox(), GetLutMap()));

		if (mFirstLight) {
			for (auto& light : Lights) {
				if (mFirstLight == light) {
					depth_state(DepthState::Make(mPerFrame.GetZFunc(kCompareLess), kDepthWriteMaskAll));
					blend_state(BlendState::MakeDisable());

					RenderLight(*mPerFrame.SetLight(light), MakePerLight(light), LIGHTMODE_FORWARD_BASE, mOpsByRT[RENDER_TYPE_GEOMETRY]);
				}
				else {
					depth_state(DepthState::Make(mPerFrame.GetZFunc(kCompareLessEqual), kDepthWriteMaskZero));
					blend_state(BlendState::MakeAdditive());

					RenderLight(*mPerFrame.SetLight(light), MakePerLight(light), LIGHTMODE_FORWARD_ADD, mOpsByRT[RENDER_TYPE_GEOMETRY]);
				}
			}
		}
	}
	void RenderPrepassBase()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		depth_state(DepthState::Make(mPerFrame.GetZFunc(kCompareLess), kDepthWriteMaskAll));
		blend_state(BlendState::MakeDisable());

		auto fb_gbuffer = mStatesBlock.LockFrameBuffer(mGBuffer, Eigen::Vector4f::Zero(), mPerFrame.GetZFar(), 0);
		fb_gbuffer.SetCallback(std::bind(&cbPerFrameBuilder::_SetFrameBuffer, mPerFrame, std::placeholders::_1));

		RenderLight(*mPerFrame.SetLight(mMainLight), MakePerLight(mMainLight), LIGHTMODE_PREPASS_BASE, mOpsByRT[RENDER_TYPE_GEOMETRY]);
	}
	void RenderPrepassFinal() 
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		auto curFB = mStatesBlock.CurrentFrameBuffer();
		BOOST_ASSERT(curFB == nullptr || curFB->GetSize() == mGBuffer->GetSize());
		mRenderSys.CopyFrameBuffer(curFB, -1, mGBuffer, -1);

		for (auto& light : Lights)
		{
			depth_state(DepthState::Make(kCompareAlways, kDepthWriteMaskZero));
			blend_state(IF_AND_OR(light == mFirstLight, BlendState::MakeAlphaNonPremultiplied(), BlendState::MakeAdditive()));

			auto attach_shadow_map = IF_AND_OR(mCfg.IsShadowVSM(), mShadowMap->GetAttachColorTexture(0), mShadowMap->GetAttachZStencilTexture());
			auto tex_shadow_map = mStatesBlock.LockTexture(kPipeTextureShadowMap, attach_shadow_map);

			auto tex_env_sheen = mStatesBlock.LockTexture(kPipeTextureEnvSheen, NULLABLE(Camera.GetSkyBox(), GetSheenMap()));
			auto tex_env_diffuse = mStatesBlock.LockTexture(kPipeTextureEnvDiffuse, NULLABLE(Camera.GetSkyBox(), GetDiffuseEnvMap()));
			auto tex_env_spec = mStatesBlock.LockTexture(kPipeTextureEnvSpec, NULLABLE(Camera.GetSkyBox(), GetTexture()));
			auto tex_env_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(Camera.GetSkyBox(), GetLutMap()));

			auto tex_gdepth = mStatesBlock.LockTexture(kPipeTextureGDepth, mGBuffer->GetAttachZStencilTexture());
			auto tex_gpos = mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(kPipeTextureGBufferPos-1));
			auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(kPipeTextureGBufferNormal-1));
			auto tex_galbedo = mStatesBlock.LockTexture(kPipeTextureGBufferAlbedo, mGBuffer->GetAttachColorTexture(kPipeTextureGBufferAlbedo-1));
			auto tex_gemissive = mStatesBlock.LockTexture(kPipeTextureGBufferEmissive, mGBuffer->GetAttachColorTexture(kPipeTextureGBufferEmissive-1));
			auto tex_gsheen = mStatesBlock.LockTexture(kPipeTextureGBufferSheen, mGBuffer->GetAttachColorTexture(kPipeTextureGBufferSheen-1));
			auto tex_gclearcoat = mStatesBlock.LockTexture(kPipeTextureGBufferClearCoat, mGBuffer->GetAttachColorTexture(kPipeTextureGBufferClearCoat-1));

			RenderLight(*mPerFrame.SetLight(light), MakePerLight(light), IF_AND_OR(light == mFirstLight, LIGHTMODE_PREPASS_FINAL, LIGHTMODE_PREPASS_FINAL_ADD), mDefferedOps);
		}
	}
	void RenderTransparent()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		depth_state(DepthState::Make(mPerFrame.GetZFunc(kCompareLess), kDepthWriteMaskAll));
		blend_state(BlendState::MakeAlphaNonPremultiplied());

		auto attach_shadow_map = IF_AND_OR(mCfg.IsShadowVSM(), mShadowMap->GetAttachColorTexture(0), mShadowMap->GetAttachZStencilTexture());
		auto tex_shadow_map = mStatesBlock.LockTexture(kPipeTextureShadowMap, attach_shadow_map);

		auto tex_env_sheen = mStatesBlock.LockTexture(kPipeTextureEnvSheen, NULLABLE(Camera.GetSkyBox(), GetSheenMap()));
		auto tex_env_diffuse = mStatesBlock.LockTexture(kPipeTextureEnvDiffuse, NULLABLE(Camera.GetSkyBox(), GetDiffuseEnvMap()));
		auto tex_env_spec = mStatesBlock.LockTexture(kPipeTextureEnvSpec, NULLABLE(Camera.GetSkyBox(), GetTexture()));
		auto tex_env_lut = mStatesBlock.LockTexture(kPipeTextureLUT, NULLABLE(Camera.GetSkyBox(), GetLutMap()));

		RenderLight(*mPerFrame, nullptr, LIGHTMODE_FORWARD_BASE, mOpsByRT[RENDER_TYPE_TRANSPARENT]);
	}
	void RenderSkybox()
	{
		auto skybox = Camera.GetSkyBox();
		if (skybox == nullptr) return;

		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();

		depth_state(DepthState::Make(mPerFrame.GetZFunc(kCompareLessEqual), kDepthWriteMaskZero));
		blend_state(BlendState::MakeDisable());

		RenderOperationQueue ops;
		skybox->GenRenderOperation(ops);
		RenderLight(*mPerFrame, nullptr, LIGHTMODE_FORWARD_BASE, ops);
	}
	void RenderOverlay()
	{
		auto depth_state = mStatesBlock.LockDepth();
		auto blend_state = mStatesBlock.LockBlend();
		
		depth_state(DepthState::MakeFor3D(false));
		blend_state(BlendState::MakeAlphaNonPremultiplied());

		RenderLight(*mPerFrame, nullptr, LIGHTMODE_OVERLAY, mOpsByRT[RENDER_TYPE_OVERLAY]);
	}
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

		DEBUG_SET_PRIV_DATA(scene_image, "_scene_image");
		mGrabDic["_SceneImage"] = scene_image;

		for (size_t i = 0; i < effects.size(); ++i) {
			auto fb_temp_output = mStatesBlock.LockFrameBuffer(tempOutputs[i]);
			fb_temp_output.SetCallback(std::bind(&cbPerFrameBuilder::_SetFrameBuffer, mPerFrame, std::placeholders::_1));
			
			auto tex_scene_img = mStatesBlock.LockTexture(kPipeTextureSceneImage, IF_AND_OR(i == 0, scene_image->GetAttachColorTexture(0), tempOutputs[i - 1]->GetAttachColorTexture(0)));
			auto tex_shadow_map = mStatesBlock.LockTexture(kPipeTextureGDepth, mGBuffer->GetAttachZStencilTexture());
			
			auto tex_gpos = mStatesBlock.LockTexture(kPipeTextureGBufferPos, mGBuffer->GetAttachColorTexture(0));
			auto tex_gnormal = mStatesBlock.LockTexture(kPipeTextureGBufferNormal, mGBuffer->GetAttachColorTexture(1));

			RenderOperationQueue ops;
			effects[i]->GenRenderOperation(ops);
			RenderLight(*mPerFrame, nullptr, LIGHTMODE_FORWARD_BASE, ops);
		}
	}
private:
	const cbPerLight* MakePerLight(const scene::LightPtr& light) {
		static cbPerLight blackLight;
		return IF_AND_OR(light, &light->GetCbLight(), &blackLight);
	}
	void EnsureGeometrySkybox(int renderType, int lightMode, std::function<void()> geoSkyRender) 
	{
		#define str_geometry_skybox "_geometry_skybox"
		if (mOpsByRT[renderType].FindPassByGrabInName(lightMode, str_geometry_skybox)) 
		{
			auto fbsize = mFbBank->GetFbSize(); fbsize.z() = -1;
			auto fbGS = mFbBank->Borrow(mFbBank->GetFbFormats(), fbsize);
			BOOST_ASSERT(fbGS->GetAttachColorCount() == 1 && fbGS->GetAttachZStencil());

			{
				auto fb_geometry_skybox = mStatesBlock.LockFrameBuffer(fbGS, Eigen::Vector4f::Zero(), mPerFrame.GetZFar(), 0);
				fb_geometry_skybox.SetCallback(std::bind(&cbPerFrameBuilder::_SetFrameBuffer, mPerFrame, std::placeholders::_1));

				geoSkyRender();
			}

			auto curFB = mStatesBlock.CurrentFrameBuffer();
			BOOST_ASSERT(curFB == nullptr || curFB->GetSize() == fbGS->GetSize());
			mRenderSys.CopyFrameBuffer(curFB, -1, fbGS, -1);
			mRenderSys.CopyFrameBuffer(curFB,  0, fbGS,  0);

			mRenderSys.GenerateMips(fbGS->GetAttachColorTexture(0));
			mPerFrame.SetLightMapSize(fbsize.head<2>(), fbGS->GetAttachColorTexture(0)->GetMipmapCount());
			DEBUG_SET_PRIV_DATA(fbGS, str_geometry_skybox);
			mGrabDic[str_geometry_skybox] = fbGS;
		}
		else 
		{
			geoSkyRender();
		}
	}
	IFrameBufferPtr QueryGrabDic(const std::string& name) 
	{
		auto iter = mTempGrabDic.find(name);
		if (iter != mTempGrabDic.end()) return iter->second;

		iter = mGrabDic.find(name);
		if (iter != mGrabDic.end()) return iter->second;

		return nullptr;
	}
	void RenderLight(cbPerFrame& perFrame, const cbPerLight* perLight, int lightMode, const RenderOperationQueue& ops)
	{
		for (const auto& op : ops) {
			perFrame.World = op.WorldTransform;
			op.WrMaterial().WriteToCb(mRenderSys, MAKE_CBNAME(cbPerFrame), Data::Make(perFrame));
			if (perLight) op.WrMaterial().WriteToCb(mRenderSys, MAKE_CBNAME(cbPerLight), Data::Make(*perLight));
			op.WrMaterial().FlushGpuParameters(mRenderSys);

			RenderOp(op, lightMode);
		}
	}
	void RenderOp(const RenderOperation& op, int lightMode)
	{
		mTempGrabDic.clear();
		res::TechniquePtr tech = op.Material->GetShader()->CurTech();
		std::vector<res::PassPtr> passes = tech->GetPassesByLightMode(lightMode);
		for (auto& pass : passes) {
			auto blend_state = mStatesBlock.LockBlend();
			const auto& blend = pass->GetBlend(); if (blend) blend_state(blend.value());

			auto depth_state = mStatesBlock.LockDepth();
			const auto& depth = pass->GetDepth(); 
			if (depth) {
				if (Camera.IsReverseZ()) {
					auto ds = depth.value();
					ds.CmpFunc = GetReverseZCompareFunc(ds.CmpFunc);
					depth_state(ds);
				}
				else {
					depth_state(depth.value());
				}
			}
			auto raster_state = mStatesBlock.LockRaster();
			const auto& cull = pass->GetCull(); if (cull) raster_state(cull.value());
			const auto& fill = pass->GetFill(); if (fill) raster_state(fill.value());
			const auto& zbias = pass->GetDepthBias(); if (zbias) raster_state(zbias.value());
			
			const auto& passOut = pass->GetGrabOut();
			if (passOut) {
				IFrameBufferPtr passFb = QueryGrabDic(passOut.Name);
				if (passFb == nullptr) {
					passFb = mFbBank->Borrow(passOut.Formats, passOut.Size);
					mTempGrabDic[passOut.Name] = passFb;
				}
				mStatesBlock.FrameBuffer.Push(passFb);
				mPerFrame.SetFrameBuffer(mStatesBlock.CurrentFrameBuffer());
			}
			const auto& passIn = pass->GetGrabIn();
			for (auto& unit : passIn) {
				IFrameBufferPtr passFb = QueryGrabDic(unit.Name);
				BOOST_ASSERT(passFb);
				if (passFb) {
					auto attach = IF_AND_OR(unit.AttachIndex >= 0, passFb->GetAttachColorTexture(unit.AttachIndex), passFb->GetAttachZStencilTexture());
					mStatesBlock.Textures(unit.TextureSlot, attach);
				}
			}

			RenderPass(pass, op);

			for (auto& unit : passIn) {
				mStatesBlock.Textures(unit.TextureSlot, nullptr);
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

		const TextureVector& textures = op.Material.GetTextures();
		if (textures.Count() > 0) {
			mStatesBlock.Textures(kTextureUserSlotFirst, &textures[0], std::min((size_t)kTextureUserSlotCount, textures.Count()));
			for (size_t slot = kTextureUserSlotLast; slot < textures.Count(); ++slot) {
				if (textures[slot]) {
					mStatesBlock.Textures(slot, textures[slot]);
				}
			}
		}
		else {
			mStatesBlock.Textures(kTextureUserSlotFirst, nullptr, 0);
		}

		const auto& relParam = pass->GetRelateToParam();
		if (relParam.HasTextureSize) {
			for (size_t slot = 0; slot < relParam.TextureSizes.size(); ++slot) {
				auto texture = mStatesBlock.Textures[slot];
				const std::string& propTexSize = relParam.TextureSizes[slot];
				if (texture && !propTexSize.empty()) {
					auto tsize = texture->GetSize();
					op.WrMaterial().SetProperty(propTexSize, Eigen::Vector4f(tsize.x(), tsize.y(), 1.0f/tsize.x(), 1.0f/tsize.y()));
				}
			}
		}

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
	const RenderableCollection& Rends;
	const scene::Camera& Camera;
	const unsigned CameraMask;
	std::vector<scene::LightPtr> Lights;
	RenderOperationQueue mDefferedOps, mCastShadowOps;
	RenderOperationQueue mOpsByRT[RENDER_TYPE_MAX];
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
	
	if (mCfg.IsShadowVSM()) mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, Eigen::Vector3i(fbSize.x(), fbSize.y(), -1), MakeResFormats(kFormatR32G32Float, kDepthFormat));
	else mShadowMap = resMng.CreateFrameBuffer(__LaunchSync__, fbSize, MakeResFormats(kFormatR8G8B8A8UNorm, kDepthFormat));
	DEBUG_SET_PRIV_DATA(mShadowMap, "render_pipeline.shadow_map");

	mGBuffer = resMng.CreateFrameBuffer(__LaunchSync__, fbSize, 
		MakeResFormats(kFormatR16G16B16A16UNorm,//Pos 
			kFormatR16G16B16A16UNorm,//Normal 
			kFormatR8G8B8A8UNorm,//Albedo 
			kFormatR8G8B8A8UNorm,//Emissive 
			kFormatR8G8B8A8UNorm,//Sheen
			kFormatR8G8B8A8UNorm,//ClearCoat
			kDepthFormat));
	DEBUG_SET_PRIV_DATA(mGBuffer, "render_pipeline.gbuffer");//Pos, Normal, Albedo, Emissive

	//mFbsBank = CreateInstance<FrameBufferBank>(resMng, fbSize, MakeResFormats(IF_AND_OR(mCfg.IsGammaSpace(), kFormatR8G8B8A8UNorm, kFormatR16G16B16A16UNorm), kDepthFormat));
	mFbsBank = CreateInstance<FrameBufferBank>(resMng, fbSize, MakeResFormats(kFormatR8G8B8A8UNorm, kDepthFormat));
}

CoTask<bool> RenderPipeline::Initialize(ResourceManager& resMng)
{
	MaterialLoadParam loadParam(MAT_DEFFERED);
	res::MaterialInstance material;
	CoAwait resMng.CreateMaterial(material, __LaunchAsync__, loadParam);

	mGBufferSprite = CreateInstance<rend::Sprite>(__LaunchAsync__, resMng, material);
	CoReturn true;
}

void RenderPipeline::SetBackColor(Eigen::Vector4f color)
{
	mBackgndColor = color;
}

void RenderPipeline::RenderCameraForward(const RenderableCollection& rends, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights)
{
	if (lights.empty()) return;

	if (camera.GetPostProcessInput())
		mStatesBlock.FrameBuffer.Push(camera.GetPostProcessInput());

	CameraRender render(*this, rends, camera, lights);
	render.Clear();
	render.RenderForwardPath();

	if (camera.GetPostProcessInput()) {
		mStatesBlock.FrameBuffer.Pop();

		render.RenderPostProcess(camera.GetPostProcessInput());
	}
}

void RenderPipeline::RenderCameraDeffered(const RenderableCollection& rends, const scene::Camera& camera, const std::vector<scene::LightPtr>& lights)
{
	if (lights.empty()) return;

	if (camera.GetPostProcessInput())
		mStatesBlock.FrameBuffer.Push(camera.GetPostProcessInput());

	CameraRender render(*this, rends, camera, lights);
	render.Clear();
	render.RenderDefferedPath();

	if (camera.GetPostProcessInput()) {
		mStatesBlock.FrameBuffer.Pop();

		render.RenderPostProcess(camera.GetPostProcessInput());
	}
}

void RenderPipeline::Render(const RenderableCollection& rends, const std::vector<scene::CameraPtr>& cameras, const std::vector<scene::LightPtr>& lights)
{
	for (auto& camera : cameras) 
	{
		auto fb_camera_output = mStatesBlock.LockFrameBuffer(IF_OR(camera->GetOutput(), nullptr));

		if (camera->GetRenderingPath() == kRenderPathForward) RenderCameraForward(rends, *camera, lights);
		else RenderCameraDeffered(rends, *camera, lights);
	}
}

bool RenderPipeline::BeginFrame()
{
	return mRenderSys.BeginScene();
}
void RenderPipeline::EndFrame()
{
	mRenderSys.EndScene();
	mFbsBank->ReturnAllTemp();
}

}