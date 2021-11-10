#pragma once
#include "core/base/std.h"
#include <boost/noncopyable.hpp>
#include "core/renderable/predeclare.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/material.h"

namespace mir {

/********** RenderOperation **********/
struct RenderOperation {
	MaterialPtr mMaterial;
	IVertexBufferPtr mVertexBuffer;
	std::map<std::pair<PassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
	IIndexBufferPtr mIndexBuffer;
	short mIndexPos = 0, mIndexCount = 0, mIndexBase = 0;
	TextureBySlot mTextures;
	XMMATRIX mWorldTransform;
public:
	RenderOperation(): mIndexPos(0), mIndexCount(0), mIndexBase(0) {
		mWorldTransform = XMMatrixIdentity();
	}
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

interface IRenderable : boost::noncopyable {
	virtual int GenRenderOperation(RenderOperationQueue& opList) = 0;
};

}