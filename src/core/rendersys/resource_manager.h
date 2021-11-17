#pragma once
#include <boost/noncopyable.hpp>
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
	void UpdateForLoading();

	IInputLayoutPtr CreateLayoutAsync(IProgramPtr pProgram, const LayoutInputElement descArray[], size_t descCount);
public:
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

	template <typename... T>
	IProgramPtr CreateProgram(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceProgram);
		return mRenderSys.LoadProgram(res, std::forward<T>(args)...);
	}

	template <typename... T>
	IInputLayoutPtr CreateLayout(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceInputLayout);
		return mRenderSys.LoadLayout(res, std::forward<T>(args)...);
	}

	template <typename... T>
	ISamplerStatePtr CreateSampler(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceSamplerState);
		return mRenderSys.LoadSampler(res, std::forward<T>(args)...);
	}

	template <typename... T>
	ITexturePtr CreateTexture(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceTexture);
		return mRenderSys.LoadTexture(res, std::forward<T>(args)...);
	}

	template <typename... T>
	IRenderTexturePtr CreateRenderTexture(T &&...args) {
		IResourcePtr res = mRenderSys.CreateResource(kDeviceResourceRenderTexture);
		return mRenderSys.LoadRenderTexture(res, std::forward<T>(args)...);
	}
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
};

}