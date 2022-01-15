#pragma once
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/assert.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
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

namespace mir {

DECLARE_STRUCT(ThreadPool);
DECLARE_STRUCT(LoadResourceJob);

typedef std::function<bool(IResourcePtr res, LoadResourceJobPtr nextJob)> LoadResourceCallback;
typedef std::function<void(IResourcePtr res)> ResourceLoadedCallback;
struct LoadResourceJob 
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void Init(Launch launchMode, LoadResourceCallback loadResCb);
	DECLARE_LAUNCH_FUNCTIONS(void, Init);
public:
	std::function<void(IResourcePtr res, LoadResourceJobPtr nextJob)> Execute;
	std::future<bool> Result;
	std::vector<unsigned char> Bytes;
	std::weak_ptr<ThreadPool> Pool;
};

class MIR_CORE_API ResourceManager : boost::noncopyable 
{
	struct ResourceLoadTaskContext {
		ResourceLoadTaskContext() {
			WorkThreadJob = CreateInstance<LoadResourceJob>();
			MainThreadJob = CreateInstance<LoadResourceJob>();
		}
		void Init(Launch launchMode, IResourcePtr res, LoadResourceCallback loadResCb, ThreadPoolPtr pool) {
			Res = res;
			WorkThreadJob->Pool = MainThreadJob->Pool = pool;
			WorkThreadJob->Init(launchMode, loadResCb);
		}
		TemplateT void AddResourceLoadedCallback(T&& cb) {
			ResLoadedCb.push_back(std::forward<T>(cb));
		}
		void FireResourceLoaded() {
			auto callbacks = std::move(ResLoadedCb);
			for (auto& cb : callbacks)
				cb(Res);
		}
	public:
		IResourcePtr Res;
		LoadResourceJobPtr WorkThreadJob, MainThreadJob;
		std::vector<ResourceLoadedCallback> ResLoadedCb;
	};
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	ResourceManager(RenderSystem& renderSys, MaterialFactory& materialFac, AiResourceFactory& aiResFac);
	~ResourceManager();
	void Dispose() ThreadSafe;
	void UpdateForLoading() ThreadSafe;
	void AddResourceDependencyRecursive(IResourcePtr to) ThreadSafe;
	void AddResourceDependency(IResourcePtr to, IResourcePtr from) ThreadSafe;//to rely-on from
	void AddLoadResourceJob(Launch launchMode, const LoadResourceCallback& loadResCb, IResourcePtr res, IResourcePtr dependRes = nullptr) ThreadSafe;
	void AddResourceLoadedObserver(IResourcePtr res, const ResourceLoadedCallback& resLoadedCB) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(void, AddLoadResourceJob, ThreadSafe);
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
		if (launchMode == LaunchAsync) {
			AddLoadResourceJobSync([=](IResourcePtr res, LoadResourceJobPtr nextJob) {
				return nullptr != mRenderSys.LoadLayout(res, program, args...);
			}, res, program);
		}
		else res->SetLoaded(nullptr != mRenderSys.LoadLayout(res, program, std::forward<T>(args)...));
		return std::static_pointer_cast<IInputLayout>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(IInputLayoutPtr, CreateLayout, ThreadSafe);

	TemplateArgs ISamplerStatePtr CreateSampler(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceSamplerState); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadSampler(res, std::forward<T>(args)...));
		return std::static_pointer_cast<ISamplerState>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(ISamplerStatePtr, CreateSampler, ThreadSafe);

	IProgramPtr CreateProgram(Launch launchMode, const std::string& name, 
		ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(IProgramPtr, CreateProgram, ThreadSafe);

	TemplateArgs ITexturePtr CreateTexture(ResourceFormat format, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceTexture);
		res->SetLoaded(nullptr != mRenderSys.LoadTexture(res, format, std::forward<T>(args)...));
		return std::static_pointer_cast<ITexture>(res);
	}
	ITexturePtr CreateTextureByFile(Launch launchMode, const std::string& filepath, 
		ResourceFormat format = kFormatUnknown, bool autoGenMipmap = false) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(ITexturePtr, CreateTextureByFile, ThreadSafe);

	TemplateArgs bool LoadRawTextureData(T &&...args) ThreadSafe {
		return mRenderSys.LoadRawTextureData(std::forward<T>(args)...);
	}

	TemplateArgs IFrameBufferPtr CreateFrameBuffer(Launch launchMode, T &&...args) ThreadSafe {
		auto res = mRenderSys.CreateResource(kDeviceResourceFrameBuffer); ResSetLaunch;
		res->SetLoaded(nullptr != mRenderSys.LoadFrameBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IFrameBuffer>(res);
	}
	DECLARE_LAUNCH_FUNCTIONS(IFrameBufferPtr, CreateFrameBuffer, ThreadSafe);

	MaterialPtr CreateMaterial(Launch launchMode, const MaterialLoadParam& matName) ThreadSafe;
	DECLARE_LAUNCH_FUNCTIONS(MaterialPtr, CreateMaterial, ThreadSafe);
	MaterialPtr CloneMaterial(Launch launchMode, const Material& material) ThreadSafe;

	AiScenePtr CreateAiScene(Launch launchMode, const std::string& assetPath, const std::string& redirectRes) ThreadSafe;
private:
	IProgramPtr _LoadProgram(IProgramPtr program, LoadResourceJobPtr nextJob, 
		const std::string& name, ShaderCompileDesc vertexSCD, ShaderCompileDesc pixelSCD) ThreadSafe;
	ITexturePtr _LoadTextureByFile(ITexturePtr texture, LoadResourceJobPtr nextJob, 
		const std::string& filepath, ResourceFormat format, bool autoGenMipmap) ThreadSafe;
private:
	RenderSystem& mRenderSys;
	MaterialFactory& mMaterialFac;
	AiResourceFactory& mAiResourceFac;
	class ResourceDependencyGraph {
		typedef IResourcePtr ValueType;
		typedef const ValueType& ConstReference;
		struct GraphNode {
			void AddLinkTo(ConstReference to) {
				auto iter = std::find(LinkTo.begin(), LinkTo.end(), to);
				if (iter == LinkTo.end())
					LinkTo.push_back(to);
			}
			void RemoveLinkTo(ConstReference to) {
				LinkTo.erase(std::remove(LinkTo.begin(), LinkTo.end(), to), LinkTo.end());
			}
			void AddLinkFrom(ConstReference from) {
				auto iter = std::find(LinkFrom.begin(), LinkFrom.end(), from);
				if (iter == LinkFrom.end())
					LinkFrom.push_back(from);
			}
			void RemoveLinkFrom(ConstReference from) {
				LinkFrom.erase(std::remove(LinkFrom.begin(), LinkFrom.end(), from), LinkFrom.end());
			}
			size_t InDgree() const { return LinkFrom.size(); }
			size_t OutDegree() const { return LinkTo.size(); }
		public:
			ValueType Value;
			std::vector<ValueType> LinkFrom;
			std::vector<ValueType> LinkTo;
		};
		GraphNode* GetGraphNode(ConstReference node) {
			auto iter = mNodeMap.find(node.get());
			return iter != mNodeMap.end() && iter->second.Value ? &iter->second : nullptr;
		}
		bool HasGraphNode(ConstReference node) const {
			auto iter = mNodeMap.find(node.get());
			return iter != mNodeMap.end() && iter->second.Value;
		}
		void AddNode(ConstReference node) {
			BOOST_ASSERT(node);
			mNodeMap[node.get()].Value = node;
		}
	public:
		void AddLink(ConstReference to, ConstReference from) {
			BOOST_ASSERT(to);
			if (from) {
				if (!HasGraphNode(to)) AddNode(to);
				if (!HasGraphNode(from)) AddNode(from);
				mNodeMap[from.get()].AddLinkTo(to);
				mNodeMap[to.get()].AddLinkFrom(from);
			}
			else {
				AddNode(to);
			}
		}
		void RemoveTopNode(ConstReference node) {
			BOOST_ASSERT(node);
			auto gnode = GetGraphNode(node);
			if (gnode) {
				BOOST_ASSERT(gnode->LinkFrom.empty());
				for (auto& to : gnode->LinkTo) {
					auto gto = GetGraphNode(to);
					if (gto) gto->RemoveLinkFrom(node);
				}
			}
			mNodeMap.erase(node.get());
		}
		template<typename _CallBack>
		void RemoveConnectedGraphByTopNode(ConstReference node, _CallBack cb) {
			BOOST_ASSERT(node);
			auto gnode = GetGraphNode(node);
			if (gnode) {
				cb(node);
				for (auto& from : gnode->LinkFrom) {
					auto gfrom = GetGraphNode(from);
					if (gfrom) gfrom->RemoveLinkTo(node);
				}
				for (auto& to : gnode->LinkTo) {
					auto gto = GetGraphNode(to);
					if (gto) gto->RemoveLinkFrom(node);
				}
				std::vector<ValueType> linkTo = std::move(gnode->LinkTo);
				mNodeMap.erase(node.get());
				for (auto& to : linkTo)
					RemoveConnectedGraphByTopNode(to, cb);
			}
		}
		const std::vector<ValueType>& GetTopNodes(std::vector<ValueType>& topNodes) {//Èë¶ÈÎª0
			topNodes.clear();
			for (auto& iter : mNodeMap) {
				if (iter.second.Value && iter.second.InDgree() == 0)
					topNodes.push_back(iter.second.Value);
			}
			return topNodes;
		}
	private:
		mutable std::map<IResourceRawPtr, GraphNode> mNodeMap;
	};
	std::vector<IResourcePtr> mRDGTopNodes;
	ResourceDependencyGraph mResDependencyGraph;
	std::map<IResourceRawPtr, ResourceLoadTaskContext> mLoadTaskCtxByRes;
	std::shared_ptr<ThreadPool> mThreadPool;
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
	std::map<ProgramKey, IProgramPtr> mProgramByKey;
	std::map<std::string, ITexturePtr> mTextureByPath;
	std::mutex mTexLock;
	std::map<MaterialLoadParam, MaterialPtr> mMaterialByName;
	struct AiResourceKey {
		std::string Path, RedirectResource;
		bool operator<(const AiResourceKey& other) const {
			if (Path != other.Path) return Path < other.Path;
			return RedirectResource < other.RedirectResource;
		}
	};
	std::map<AiResourceKey, AiScenePtr> mAiSceneByKey;
	std::atomic<bool> mProgramMapLock, mTextureMapLock, mMaterialMapLock, mAiSceneMapLock;
	std::atomic<bool> mLoadTaskCtxMapLock, mResDependGraphLock;
};

}