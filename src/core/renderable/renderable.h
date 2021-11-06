#pragma once
#include "core/base/std.h"
#include "core/renderable/renderable_pred.h"
#include "core/rendersys/material.h"
#include "core/rendersys/interface_type_pred.h"

namespace mir {

/********** RenderOperation **********/
struct RenderOperation {
	TMaterialPtr mMaterial;
	IVertexBufferPtr mVertexBuffer;
	std::map<std::pair<TPassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
	IIndexBufferPtr mIndexBuffer;
	short mIndexPos = 0, mIndexCount = 0, mIndexBase = 0;
	TextureBySlot mTextures;
	XMMATRIX mWorldTransform;
public:
	RenderOperation();
};

struct RenderOperationQueue {
	std::vector<RenderOperation> mOps;
public:
	void Clear();
	void AddOP(const RenderOperation& op);
	size_t Count() const;
	RenderOperation& At(size_t pos);
	const RenderOperation& At(size_t pos) const;
	RenderOperation& operator[](size_t pos);
	const RenderOperation& operator[](size_t pos) const;
};

interface IRenderable {
	virtual int GenRenderOperation(RenderOperationQueue& opList) = 0;
};

}