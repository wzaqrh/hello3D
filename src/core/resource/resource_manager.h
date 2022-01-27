#pragma once
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/assert.hpp>
#include "core/mir_export.h"
#include "core/base/tpl/atomic_map.h"
#include "core/base/stl.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/declare_macros.h"
#include "core/base/material_load_param.h"
#include "core/predeclare.h"
#include "core/rendersys/blob.h"
#include "core/rendersys/program.h"
#include "core/rendersys/input_layout.h"
#include "core/rendersys/hardware_buffer.h"
#include "core/rendersys/texture.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/render_system.h"
#include "core/resource/material_name.h"
#include "core/resource/material.h"

namespace mir {
DECLARE_STRUCT(ThreadPool);
DECLARE_STRUCT(LoadResourceJob);
DECLARE_STRUCT(LoadResourceJob2);

class MIR_CORE_API ResourceManager : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	ResourceManager(RenderSystem& renderSys, res::MaterialFactory& materialFac, res::AiResourceFactory& aiResFac, std::shared_ptr<cppcoro::io_service> ioService);
	~ResourceManager();
	void Dispose() ThreadSafe;
	void UpdateForLoading() ThreadSafe;
public:
	RenderSystem& RenderSys() { return mRenderSys; }
	Eigen::Vector2i WinSize() const { return mRenderSys.WinSize(); }

	TemplateArgs IIndexBufferPtr CreateIndexBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceIndexBuffer); ResSetLaunch;
		mRenderSys.LoadIndexBuffer(res, std::forward<T>(args)...);
		res->SetLoaded();
		return std::static_pointer_cast<IIndexBuffer>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(IIndexBufferPtr, CreateIndexBuffer, ThreadSafe);

	TemplateArgs IVertexBufferPtr CreateVertexBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceVertexBuffer); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadVertexBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IVertexBuffer>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(IVertexBufferPtr, CreateVertexBuffer, ThreadSafe);

	TemplateArgs IContantBufferPtr CreateConstBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceContantBuffer); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadConstBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IContantBuffer>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(IContantBufferPtr, CreateConstBuffer, ThreadSafe);

	TemplateArgs bool UpdateBuffer(T &&...args) ThreadSafe {
		return mRenderSys.UpdateBuffer(std::forward<T>(args)...);
	}

	TemplateArgs IInputLayoutPtr CreateLayout(Launch launchMode, IProgramPtr program, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceInputLayout); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadLayout(res, program, std::forward<T>(args)...));
		return std::static_pointer_cast<IInputLayout>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(IInputLayoutPtr, CreateLayout, ThreadSafe);

	TemplateArgs ISamplerStatePtr CreateSampler(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceSamplerState); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadSampler(res, std::forward<T>(args)...));
		return std::static_pointer_cast<ISamplerState>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(ISamplerStatePtr, CreateSampler, ThreadSafe);

	cppcoro::shared_task<IProgramPtr> CreateProgram(Launch launchMode, const std::string& name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(cppcoro::shared_task<IProgramPtr>, CreateProgram, ThreadSafe);

	TemplateArgs ITexturePtr CreateTexture(ResourceFormat format, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceTexture);
		res->SetLoaded(nullptr != mRenderSys.LoadTexture(res, format, std::forward<T>(args)...));
		return std::static_pointer_cast<ITexture>(res);
	}
	cppcoro::shared_task<ITexturePtr> CreateTextureByFile(Launch launchMode, const std::string& filepath, 
		ResourceFormat format = kFormatUnknown, bool autoGenMipmap = false) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(cppcoro::shared_task<ITexturePtr>, CreateTextureByFile, ThreadSafe);

	TemplateArgs bool LoadRawTextureData(T &&...args) ThreadSafe {
		return mRenderSys.LoadRawTextureData(std::forward<T>(args)...);
	}

	TemplateArgs IFrameBufferPtr CreateFrameBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceFrameBuffer); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadFrameBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IFrameBuffer>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(IFrameBufferPtr, CreateFrameBuffer, ThreadSafe);

	cppcoro::shared_task<res::ShaderPtr> CreateShader(Launch launchMode, const MaterialLoadParam& loadParam) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(cppcoro::shared_task<res::ShaderPtr>, CreateShader, ThreadSafe);

	cppcoro::shared_task<res::MaterialInstance> CreateMaterial(Launch launchMode, const MaterialLoadParam& loadParam) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(cppcoro::shared_task<res::MaterialInstance>, CreateMaterial, ThreadSafe);

	cppcoro::shared_task<res::AiScenePtr> CreateAiScene(Launch launchMode, const std::string& assetPath, const std::string& redirectRes) ThreadSafe;
public:
	cppcoro::static_thread_pool& GetAsyncService() { return *mThreadPool; }
	cppcoro::io_service& GetSyncService() { return *mIoService; }
	bool IsCurrentInAsyncService() const;
	cppcoro::shared_task<void> SwitchToLaunchService(Launch launchMode);
private:
	cppcoro::shared_task<bool> _LoadProgram(IProgramPtr program, Launch launchMode, const std::string& name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe;
	cppcoro::shared_task<bool> _LoadTextureByFile(ITexturePtr texture, Launch launchMode, const std::string& filepath, ResourceFormat format, bool autoGenMipmap) ThreadSafe;
private:
	RenderSystem& mRenderSys;
	res::MaterialFactory& mMaterialFac;
	res::AiResourceFactory& mAiResourceFac;
	std::shared_ptr<cppcoro::static_thread_pool> mThreadPool;
	std::shared_ptr<cppcoro::io_service> mIoService;
	std::thread::id mMainThreadId;
private:
	std::vector<unsigned char> mTempBytes;
	struct ProgramKey {
		std::string name;
		ShaderCompileDesc vertexSCD, pixelSCD;
		bool operator<(const ProgramKey& other) const {
			if (name != other.name) return name < other.name;
			if (!(vertexSCD == other.vertexSCD)) return vertexSCD < other.vertexSCD;
			return pixelSCD < other.pixelSCD;
		}
	};
	tpl::AtomicMap<ProgramKey, IProgramPtr> mProgramByKey;
	tpl::AtomicMap<std::string, ITexturePtr> mTextureByKey;
	tpl::AtomicMap<MaterialLoadParam, res::ShaderPtr> mShaderByName;
	tpl::AtomicMap<MaterialLoadParam, res::MaterialPtr> mMaterialByName;
	struct AiResourceKey {
		std::string Path, RedirectResource;
		bool operator<(const AiResourceKey& other) const {
			if (Path != other.Path) return Path < other.Path;
			return RedirectResource < other.RedirectResource;
		}
	};
	tpl::AtomicMap<AiResourceKey, res::AiScenePtr> mAiSceneByKey;
};

}