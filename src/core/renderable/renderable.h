#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/renderable/predeclare.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/material.h"

namespace mir {

/********** RenderOperation **********/
struct RenderOperation {
	RenderOperation() : mIndexPos(0), mIndexCount(0), mIndexBase(0), mWorldTransform(Eigen::Matrix4f::Identity()) {}
	template<class T> void SetConstantBufferBytes(const std::string& cbName, const T& value) {
		mCBDataByName[cbName].assign((char*)&value, (char*)&value + sizeof(value));
	}
public:
	MaterialPtr mMaterial;
	IVertexBufferPtr mVertexBuffer;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
	IIndexBufferPtr mIndexBuffer;
	short mIndexPos, mIndexCount, mIndexBase;
	TextureBySlot mTextures;
	Eigen::Matrix4f mWorldTransform;
	typedef std::function<void(RenderSystem& renderSys, const Pass& pass, const RenderOperation& op)> PassCallback;
	PassCallback OnBind, OnUnbind;
	std::map<std::string, std::vector<char>> mCBDataByName;
};

struct RenderOperationQueue {
	std::vector<RenderOperation> mOps;
public:
	void Clear() {
		mOps.clear();
	}
	void AddOP(const RenderOperation& op) {
		mOps.push_back(op);
	}
	size_t Count() const { return mOps.size(); }
	bool IsEmpty() const { return mOps.empty(); }

	RenderOperation& At(size_t pos) { return mOps[pos]; }
	RenderOperation& operator[](size_t pos) { return At(pos); }

	const RenderOperation& At(size_t pos) const { return mOps[pos]; }
	const RenderOperation& operator[](size_t pos) const { return At(pos); }
};

interface MIR_CORE_API IRenderable : boost::noncopyable {
	virtual int GenRenderOperation(RenderOperationQueue& opList) = 0;
};

}