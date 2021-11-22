#pragma once
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/assert.hpp>
//#include <boost/asio/thread_pool.hpp> 
//#include <boost/asio/io_service.hpp>
//#include <boost/thread/thread.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/base/launch.h"
#include "core/rendersys/resource.h"
#include "core/rendersys/render_system.h"

namespace mir {

class MIR_CORE_API ResourceManager : boost::noncopyable {
	typedef std::function<bool(IResourcePtr)> ResourceLoadTask;
public:
	ResourceManager(RenderSystem& renderSys, MaterialFactory& materialFac);
	~ResourceManager();
	void UpdateForLoading();
	void AddResourceDependency(IResourcePtr to, IResourcePtr from);//parent rely-on node
public:
	Eigen::Vector2i WinSize() const { return mRenderSys.WinSize(); }

	TemplateArgs IIndexBufferPtr CreateIndexBuffer(Launch launchMode, T &&...args) {
		auto res = mRenderSys.CreateResource(kDeviceResourceIndexBuffer);
		mRenderSys.LoadIndexBuffer(res, std::forward<T>(args)...);
		res->SetLoaded();
		return std::static_pointer_cast<IIndexBuffer>(res);
	}
	DECLARE_RES_MNG_LAUNCH_FUNCS(IIndexBufferPtr, CreateIndexBuffer);

	TemplateArgs IVertexBufferPtr CreateVertexBuffer(Launch launchMode, T &&...args) {
		auto res = mRenderSys.CreateResource(kDeviceResourceVertexBuffer);
		res->SetLoaded(nullptr != mRenderSys.LoadVertexBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IVertexBuffer>(res);
	}
	DECLARE_RES_MNG_LAUNCH_FUNCS(IVertexBufferPtr, CreateVertexBuffer);

	TemplateArgs IContantBufferPtr CreateConstBuffer(Launch launchMode, T &&...args) {
		auto res = mRenderSys.CreateResource(kDeviceResourceContantBuffer);
		res->SetLoaded(nullptr != mRenderSys.LoadConstBuffer(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IContantBuffer>(res);
	}
	DECLARE_RES_MNG_LAUNCH_FUNCS(IContantBufferPtr, CreateConstBuffer);

	TemplateArgs bool UpdateBuffer(T &&...args) {
		return mRenderSys.UpdateBuffer(std::forward<T>(args)...);
	}

	TemplateArgs IInputLayoutPtr CreateLayout(Launch launchMode, IProgramPtr program, T &&...args) {
		auto res = mRenderSys.CreateResource(kDeviceResourceInputLayout);
		if (launchMode == Launch::Async) {
			AddResourceDependency(res, program);
			res->SetPrepared();
			mLoadTaskByRes[res] = [=](IResourcePtr res) { 
				return nullptr != mRenderSys.LoadLayout(res, program, args...);
			};
		}
		else res->SetLoaded(nullptr != mRenderSys.LoadLayout(res, program, std::forward<T>(args)...));
		return std::static_pointer_cast<IInputLayout>(res);
	}
	DECLARE_RES_MNG_LAUNCH_FUNCS(IInputLayoutPtr, CreateLayout);

	TemplateArgs ISamplerStatePtr CreateSampler(Launch launchMode, T &&...args) {
		auto res = mRenderSys.CreateResource(kDeviceResourceSamplerState);
		res->SetLoaded(nullptr != mRenderSys.LoadSampler(res, std::forward<T>(args)...));
		return std::static_pointer_cast<ISamplerState>(res);
	}
	DECLARE_RES_MNG_LAUNCH_FUNCS(ISamplerStatePtr, CreateSampler);

	IProgramPtr CreateProgram(Launch launchMode, const std::string& name, const std::string& vsEntry, const std::string& psEntry);
	DECLARE_RES_MNG_LAUNCH_FUNCS(IProgramPtr, CreateProgram);

	TemplateArgs ITexturePtr CreateTexture(ResourceFormat format, T &&...args) {
		auto res = mRenderSys.CreateResource(kDeviceResourceTexture);
		res->SetLoaded(nullptr != mRenderSys.LoadTexture(res, format, std::forward<T>(args)...));
		return std::static_pointer_cast<ITexture>(res);
	}
	ITexturePtr CreateTextureByFile(Launch launchMode, const std::string& filepath, ResourceFormat format = kFormatUnknown, bool autoGenMipmap = false);
	DECLARE_RES_MNG_LAUNCH_FUNCS(ITexturePtr, CreateTextureByFile);

	TemplateArgs bool LoadRawTextureData(T &&...args) {
		return mRenderSys.LoadRawTextureData(std::forward<T>(args)...);
	}

	TemplateArgs IRenderTexturePtr CreateRenderTexture(Launch launchMode, T &&...args) {
		auto res = mRenderSys.CreateResource(kDeviceResourceRenderTexture);
		res->SetLoaded(nullptr != mRenderSys.LoadRenderTexture(res, std::forward<T>(args)...));
		return std::static_pointer_cast<IRenderTexture>(res);
	}
	DECLARE_RES_MNG_LAUNCH_FUNCS(IRenderTexturePtr, CreateRenderTexture);

	MaterialPtr CreateMaterial(Launch launchMode, const std::string& matName, bool sharedUse = false);
	DECLARE_RES_MNG_LAUNCH_FUNCS(MaterialPtr, CreateMaterial);

	MaterialPtr CloneMaterial(Launch launchMode, const Material& material);
private:
	IProgramPtr _LoadProgram(IProgramPtr program, const std::string& name, const std::string& vsEntry, const std::string& psEntry);
	ITexturePtr _LoadTextureByFile(ITexturePtr texture, const std::string& filepath, ResourceFormat format, bool autoGenMipmap);
private:
	RenderSystem& mRenderSys;
	MaterialFactory& mMaterialFac;
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
		const std::vector<ValueType>& GetTopNodes() {//Èë¶ÈÎª0
			mTopNodes.clear();
			for (auto& iter : mNodeMap) {
				if (iter.second.Value && iter.second.InDgree() == 0)
					mTopNodes.push_back(iter.second.Value);
			}
			return mTopNodes;
		}
	private:
		mutable std::map<IResourceRawPtr, GraphNode> mNodeMap;
		mutable std::vector<ValueType> mTopNodes;
	};
	ResourceDependencyGraph mResDependencyGraph;
	std::map<IResourcePtr, ResourceLoadTask> mLoadTaskByRes;
	//std::vector<std::tuple<IResourcePtr, std::future<bool>>> mLoadingTasks;
	//boost::asio::thread_pool mThreadPool;
private:
	std::vector<unsigned char> mTempBytes;
	struct ProgramKey {
		std::string name, vsEntry, psEntry;
		bool operator<(const ProgramKey& other) const {
			if (name != other.name) return name < other.name;
			if (vsEntry != other.vsEntry) return vsEntry < other.vsEntry;
			return psEntry < other.psEntry;
		}
	};
	std::map<ProgramKey, IProgramPtr> mProgramByKey;
	std::map<std::string, ITexturePtr> mTexByPath;
	std::map<std::string, MaterialPtr> mMaterials;
};

}