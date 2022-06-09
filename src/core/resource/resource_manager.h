#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/predeclare.h"
#include "core/rendersys/render_system.h"
#include "core/resource/material.h"
#include "core/resource/device_res_factory.h"

namespace mir {

class MIR_CORE_API ResourceManager : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	ResourceManager(RenderSystem& renderSys, std::shared_ptr<cppcoro::io_service> ioService);
	~ResourceManager();
	void Dispose() ThreadSafe;
	CoTask<void> UpdateFrame(float dt) ThreadSafe;
public:
	Eigen::Vector2i WinSize() const { return mRenderSys.WinSize(); }
	int WinWidth() const { return WinSize().x(); }
	int WinHeight() const { return WinSize().y(); }

#define TRANSMIT_MEMFAC_FUNCTION(FAC, RETURN_TYPE, FUNC_NAME) TemplateArgs RETURN_TYPE FUNC_NAME(T &&...args) ThreadSafe { return FAC->FUNC_NAME(std::forward<T>(args)...); }
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, IIndexBufferPtr, CreateIndexBuffer, ThreadSafe);
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, IVertexBufferPtr, CreateVertexBuffer, ThreadSafe); 
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, IContantBufferPtr, CreateConstBuffer, ThreadSafe); 
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, bool, UpdateBuffer, ThreadSafe);
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, IInputLayoutPtr, CreateLayout, ThreadSafe);
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, ISamplerStatePtr, CreateSampler, ThreadSafe);
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, ITexturePtr, CreateTexture, ThreadSafe);
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac, bool, LoadRawTextureData, ThreadSafe);
	TRANSMIT_MEMFAC_FUNCTION(mDeviceResFac,IFrameBufferPtr, CreateFrameBuffer, ThreadSafe);

#define TRANSMIT_MEMFAC_TASK_FUNCTION(FAC, RETURN_TYPE, FUNC_NAME) DECLARE_COTASK_FUNCTIONS(RETURN_TYPE, FUNC_NAME, ThreadSafe)\
	TemplateArgs CoTask<bool> FUNC_NAME(T &&...args) ThreadSafe { return FAC->FUNC_NAME(std::forward<T>(args)...); }
	TRANSMIT_MEMFAC_TASK_FUNCTION(mTextureFac, ITexturePtr, CreateTextureByData, ThreadSafe);
	TRANSMIT_MEMFAC_TASK_FUNCTION(mTextureFac, ITexturePtr, CreateTextureByFile, ThreadSafe);
	TRANSMIT_MEMFAC_TASK_FUNCTION(mProgramFac, IProgramPtr, CreateProgram, ThreadSafe);
	TRANSMIT_MEMFAC_TASK_FUNCTION(mMaterialFac, res::MaterialInstance, CreateMaterial, ThreadSafe);
	TRANSMIT_MEMFAC_TASK_FUNCTION(mMaterialFac, res::ShaderPtr, CreateShader, ThreadSafe);
	TRANSMIT_MEMFAC_TASK_FUNCTION(mAiResFac, res::AiScenePtr, CreateAiScene, ThreadSafe);
public:
	RenderSystem& RenderSys() { return mRenderSys; }
	res::MaterialFactory& GetMtlFac() { return *mMaterialFac; }
	res::ProgramFactory& GetProgramFac() { return *mProgramFac; }
	res::TextureFactory& GetTexFac() { return *mTextureFac; }
	res::AiResourceFactory& GetAiResFac() { return *mAiResFac; }
	res::DeviceResFactory& GetDeviceResFac() { return *mDeviceResFac; }

	bool IsCurrentInAsyncService() const;
	CoTask<void> SwitchToLaunchService(Launch launchMode);
	CoTask<void> WaitResComplete(IResourcePtr res, std::chrono::microseconds interval = std::chrono::microseconds(1));
private:
	RenderSystem& mRenderSys;
	res::DeviceResFactoryPtr mDeviceResFac;
	res::TextureFactoryPtr mTextureFac;
	res::ProgramFactoryPtr mProgramFac;
	res::MaterialFactoryPtr mMaterialFac;
	res::AiResourceFactoryPtr mAiResFac;
	

	std::shared_ptr<cppcoro::static_thread_pool> mThreadPool;
	std::shared_ptr<cppcoro::io_service> mIoService;
	std::thread::id mMainThreadId;
};

}