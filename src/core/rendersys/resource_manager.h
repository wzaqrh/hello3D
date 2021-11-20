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
	ITexturePtr CreateTexture(const std::string& filepath, ResourceFormat format = kFormatUnknown, bool autoGenMipmap = false);

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
	ITexturePtr DoCreateTexture(const std::string& filepath, ResourceFormat format, bool autoGenMipmap);
private:
	RenderSystem& mRenderSys;
	struct ResourceDependencyTree {
		typedef IResourcePtr ValueType;
		typedef const ValueType& ConstReference;
		std::map<ValueType, ValueType> mParentMap;
		mutable std::map<ValueType, std::vector<ValueType>> mChildrenMap;
		mutable std::vector<ValueType> mTopNodes;
		ConstReference NodeParent(ConstReference node) const {
			auto iter = mParentMap.find(node);
			return (iter != mParentMap.end()) ? iter->second : nullptr;
		}
		const std::vector<ValueType>& NodeChildren(ConstReference node) const {
			return mChildrenMap[node];
		}
		void AddNode(ConstReference node, ConstReference parent) {
			if (HasNode(parent)) {
				mChildrenMap[parent].push_back(node);
				mParentMap[node] = parent;
			}
			else {
				mParentMap[node] = nullptr;
			}
		}
		void RemoveNode(ConstReference node) {
			ValueType parent = NodeParent(node);
			if (parent != nullptr) {
				auto& children = mChildrenMap[parent];
				children.erase(std::remove(children.begin(), children.end(), node), children.end());
			}
		}
		bool HasNode(ConstReference node) const {
			return mParentMap.find(node) != mParentMap.end();
		}
		const std::vector<ValueType>& TopNodes() {
			mTopNodes.clear();
			for (auto& iter : mParentMap) {
				ConstReference node = iter.first;
				if (mChildrenMap[node].empty())
					mTopNodes.push_back(node);
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