#pragma once
#include "std.h"
#include "TPredefine.h"

/********** RenderOperation **********/
struct TTextureBySlot {
	std::vector<ITexturePtr> textures;
public:
	void clear();
	void push_back(ITexturePtr texture);
	bool empty() const;
	size_t size() const;
	void swap(TTextureBySlot& other);
	void resize(size_t size);
	const ITexturePtr At(size_t pos) const;
	ITexturePtr& At(size_t pos);
	const ITexturePtr operator[](size_t pos) const;
	ITexturePtr& operator[](size_t pos);
	std::vector<ID3D11ShaderResourceView*> GetTextureViews() const;
	void Merge(const TTextureBySlot& other);
};
typedef std::shared_ptr<TTextureBySlot> TTextureBySlotPtr;

struct TRenderOperation {
	TMaterialPtr mMaterial;
	IVertexBufferPtr mVertexBuffer;
	std::map<std::pair<TPassPtr, int>, IVertexBufferPtr> mVertBufferByPass;
	IIndexBufferPtr mIndexBuffer;
	TTextureBySlot mTextures;
	XMMATRIX mWorldTransform;
public:
	TRenderOperation();
};

struct TRenderOperationQueue {
	std::vector<TRenderOperation> mOps;
public:
	void clear();
	void push_back(const TRenderOperation& op);
	size_t size() const;
	TRenderOperation& At(size_t pos);
	const TRenderOperation& At(size_t pos) const;
	TRenderOperation& operator[](size_t pos);
	const TRenderOperation& operator[](size_t pos) const;
};

struct IRenderable {
	virtual int GenRenderOperation(TRenderOperationQueue& opList) = 0;
};
