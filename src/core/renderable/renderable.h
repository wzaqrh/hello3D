#pragma once
#include "std.h"
#include "IRenderablePred.h"
#include "TMaterial.h"
#include "TInterfaceTypePred.h"

/********** RenderOperation **********/
struct TRenderOperation {
	TMaterialPtr mMaterial;
	IVertexBufferPtr mVertexBuffer;
	std::map<std::pair<TPassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
	IIndexBufferPtr mIndexBuffer;
	short mIndexPos = 0, mIndexCount = 0, mIndexBase = 0;
	TTextureBySlot mTextures;
	XMMATRIX mWorldTransform;
public:
	TRenderOperation();
};

struct TRenderOperationQueue {
	std::vector<TRenderOperation> mOps;
public:
	void Clear();
	void AddOP(const TRenderOperation& op);
	size_t Count() const;
	TRenderOperation& At(size_t pos);
	const TRenderOperation& At(size_t pos) const;
	TRenderOperation& operator[](size_t pos);
	const TRenderOperation& operator[](size_t pos) const;
};

struct IRenderable {
	virtual int GenRenderOperation(TRenderOperationQueue& opList) = 0;
};
