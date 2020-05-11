#pragma once
#include "std.h"
#include "TPredefine.h"

/********** RenderOperation **********/
struct TTextureBySlot {
	std::vector<ITexturePtr> textures;
public:
	bool empty() const;
	size_t size() const;
	
	void clear();
	void push_back(ITexturePtr texture);
	void swap(TTextureBySlot& other);
	void resize(size_t size);

	const ITexturePtr At(size_t pos) const;
	ITexturePtr& At(size_t pos);
	
	const ITexturePtr operator[](size_t pos) const;
	ITexturePtr& operator[](size_t pos);
public:
	void Merge(const TTextureBySlot& other);
};
typedef std::shared_ptr<TTextureBySlot> TTextureBySlotPtr;

struct TRenderOperation {
	TMaterialPtr mMaterial;
	IVertexBufferPtr mVertexBuffer;
	std::map<std::pair<TPassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
	IIndexBufferPtr mIndexBuffer;
	short mIndexPos, mIndexCount, mIndexBase;
	TTextureBySlot mTextures;
	XMMATRIX mWorldTransform;
public:
	TRenderOperation();
};

MIDL_INTERFACE("36718746-09D1-444E-B38D-21C61ECDEBA4")
IRenderOperationQueue : public IUnknown{

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
