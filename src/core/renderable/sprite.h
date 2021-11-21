#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

struct SpriteVertex {
	Eigen::Vector3f Pos;
	unsigned int Color;
	Eigen::Vector2f Tex;
};

struct SpriteVertexQuad {
	SpriteVertexQuad();
	SpriteVertexQuad(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetColor(const Eigen::Vector4f& color);
	void SetZ(float z);
	void FlipY();
	void SetTexCoord(const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1);
private:
	void DoSetTexCoords(Eigen::Vector2f plb, Eigen::Vector2f prt);
public:
	SpriteVertex lb, lt, rt, rb;
};

class MIR_CORE_API Sprite : public IRenderable 
{
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Sprite);
	Sprite(ResourceManager& resourceMng, const std::string& matName = "");
public:
	~Sprite();
	void SetPosition(const Eigen::Vector3f& pos);
	void SetSize(const Eigen::Vector2f& size);
	void SetTexture(const ITexturePtr& Texture);
	void SetColor(const Eigen::Vector4f& color);
	void SetFlipY(bool flipY);
public:
	int GenRenderOperation(RenderOperationQueue& opList) override;
	const SpriteVertexQuad* GetVertexData() const { return &mQuad; }
	const MaterialPtr& GetMaterial() const { return mMaterial; }
	const TransformPtr& GetTransform() const { return mTransform; }
private:
	ResourceManager& mResourceMng;
	ITexturePtr mTexture = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
	TransformPtr mTransform;
private:
	SpriteVertexQuad mQuad;
	bool mQuadDirty;
	Eigen::Vector2f mPosition;
	Eigen::Vector2f mSize;
	bool mFlipY;
};

}