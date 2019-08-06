#pragma once
#include "std.h"
#include "TPredefine.h"

/********** RenderOperation **********/
struct TTextureBySlot {
	std::vector<TTexturePtr> textures;
public:
	void clear();
	void push_back(TTexturePtr texture);
	bool empty() const;
	size_t size() const;
	void swap(TTextureBySlot& other);
	void resize(size_t size);
	const TTexturePtr At(size_t pos) const;
	TTexturePtr& At(size_t pos);
	const TTexturePtr operator[](size_t pos) const;
	TTexturePtr& operator[](size_t pos);
	std::vector<ID3D11ShaderResourceView*> GetTextureViews() const;
	void Merge(const TTextureBySlot& other);
};
typedef std::shared_ptr<TTextureBySlot> TTextureBySlotPtr;

struct TRenderOperation {
	TMaterialPtr mMaterial;
	TVertexBufferPtr mVertexBuffer;
	std::map<std::pair<TPassPtr, int>, TVertexBufferPtr> mVertBufferByPass;
	TIndexBufferPtr mIndexBuffer;
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
