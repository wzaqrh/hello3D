#pragma once
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/assert.hpp>
#include "core/base/stl.h"
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/resource.h"
#include "core/rendersys/render_system.h"

namespace mir {

class MIR_CORE_API ResourceManager : boost::noncopyable {
	typedef std::function<void(IResourcePtr)> ResourceLoadTask;
public:
	ResourceManager(RenderSystem& renderSys);
	~ResourceManager();
	void UpdateForLoading();
	void AddResourceDependency(IResourcePtr node, IResourcePtr parent);
public:
	Eigen::Vector4i WinSize() const { return mRenderSys.WinSize(); }

	template <typename... T> 
	IIndexBufferPtr CreateIndexBuffer(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceIndexBuffer);
		return mRenderSys.LoadIndexBuffer(res, std::forward<T>(args)...);
	}

	template <typename... T>
	IVertexBufferPtr CreateVertexBuffer(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceVertexBuffer);
		return mRenderSys.LoadVertexBuffer(res, std::forward<T>(args)...);
	}

	template <typename... T>
	IContantBufferPtr CreateConstBuffer(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceContantBuffer);
		return mRenderSys.LoadConstBuffer(res, std::forward<T>(args)...);
	}
	template <typename... T>
	bool UpdateBuffer(T &&...args) {
		return mRenderSys.UpdateBuffer(std::forward<T>(args)...);
	}

	IProgramPtr CreateProgram(const std::string& name, const std::string& vsEntry, const std::string& psEntry);
	template <typename... T>
	IProgramPtr CreateProgramAsync(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceProgram);
		AddResourceDependency(res, nullptr);

		mLoadTaskByRes[res] = [=](IResourcePtr res) {
			mRenderSys.LoadProgram(res, args...);
		};
		return std::static_pointer_cast<IProgram>(res);
	}

	template <typename... T>
	IInputLayoutPtr CreateLayout(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceInputLayout);
		return mRenderSys.LoadLayout(res, std::forward<T>(args)...);
	}
	template <typename... T>
	IInputLayoutPtr CreateLayoutAsync(IProgramPtr program, T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceInputLayout);
		AddResourceDependency(res, program);

		mLoadTaskByRes[res] = [=](IResourcePtr res) {
			mRenderSys.LoadLayout(res, program, args...);
		};
		return std::static_pointer_cast<IInputLayout>(res);
	}

	template <typename... T>
	ISamplerStatePtr CreateSampler(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceSamplerState);
		return mRenderSys.LoadSampler(res, std::forward<T>(args)...);
	}

	template <typename... T>
	ITexturePtr CreateTexture(ResourceFormat format, T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceTexture);
		return mRenderSys.LoadTexture(res, format, std::forward<T>(args)...);
	}
	template <typename... T>
	ITexturePtr CreateTextureByFile(T &&...args) {
		return _CreateTextureByFile(false, std::forward<T>(args)...);
	}
	template <typename... T>
	ITexturePtr CreateTextureByFileAsync(T &&...args) {
		return _CreateTextureByFile(true, std::forward<T>(args)...);
	}

	template <typename... T>
	bool LoadRawTextureData(T &&...args) {
		return mRenderSys.LoadRawTextureData(std::forward<T>(args)...);
	}

	template <typename... T>
	IRenderTexturePtr CreateRenderTexture(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceRenderTexture);
		return mRenderSys.LoadRenderTexture(res, std::forward<T>(args)...);
	}
private:
	ITexturePtr _CreateTextureByFile(bool async, const std::string& filepath, ResourceFormat format = kFormatUnknown, bool autoGenMipmap = false);
	ITexturePtr _LoadTextureByFile(ITexturePtr texture, const std::string& filepath, ResourceFormat format, bool autoGenMipmap);
private:
	RenderSystem& mRenderSys;
	struct ResourceDependencyTree {
		typedef IResourcePtr ValueType;
		typedef const ValueType& ConstReference;
		struct TreeNode {
			ValueType Parent;
			ValueType Value;
			std::vector<ValueType> Children;
		};
		mutable std::map<IResourceRawPtr, TreeNode> mNodeMap;
		mutable std::vector<ValueType> mTopNodes;
		ValueType NodeParent(ConstReference node) const {
			auto iter = mNodeMap.find(node.get());
			return (iter != mNodeMap.end() && iter->second.Parent) ? iter->second.Parent : nullptr;
		}
		const std::vector<ValueType>& NodeChildren(ConstReference node) const {
			return mNodeMap[node.get()].Children;
		}
		bool HasNode(ConstReference node) const {
			auto iter = mNodeMap.find(node.get());
			return iter != mNodeMap.end() && iter->second.Value;
		}
		void AddNode(ConstReference node, ConstReference parent) {
			if (HasNode(parent)) {
				mNodeMap[parent.get()].Children.push_back(node);
				mNodeMap[node.get()].Parent = parent;
				mNodeMap[node.get()].Value = node;
			}
			else {
				mNodeMap[node.get()].Value = node;
			}
		}
		void RemoveNode(ConstReference node) {
			mNodeMap.erase(node.get());
		}
		const std::vector<ValueType>& TopNodes() {
			mTopNodes.clear();
			for (auto& iter : mNodeMap) {
				if (iter.second.Value && mNodeMap[iter.first].Children.empty())
					mTopNodes.push_back(iter.second.Value);
			}
			return mTopNodes;
		}
	};
	ResourceDependencyTree mResDependencyTree;
	std::map<IResourcePtr, ResourceLoadTask> mLoadTaskByRes;
private:
	std::map<std::string, ITexturePtr> mTexByPath;
	std::vector<unsigned char> mTempBytes;
};

}