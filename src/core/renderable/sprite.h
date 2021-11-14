#pragma once
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

struct SpriteVertex {
	Eigen::Vector3f Pos;
	unsigned int Color;
	Eigen::Vector2f Tex;
};

struct SpriteVertexQuad {
	SpriteVertex lb, lt, rt, rb;
	SpriteVertexQuad();
	SpriteVertexQuad(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetColor(const Eigen::Vector4f& color);
	void SetZ(float z);
	void FlipY();
	void SetTexCoord(const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1);
private:
	void DoSetTexCoords(Eigen::Vector2f plb, Eigen::Vector2f prt);
};

class MIR_CORE_API Sprite : public IRenderable 
{
public:
	Sprite(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& matName = "");
	~Sprite();
	void SetPosition(const Eigen::Vector3f& pos);
	void SetSize(const Eigen::Vector2f& size);
	void SetTexture(ITexturePtr Texture);
	void SetColor(const Eigen::Vector4f& color);
	void SetFlipY(bool flipY);
public:
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
	const SpriteVertexQuad* GetQuad() const { return &mQuad; }
public:
	MaterialPtr mMaterial;
	TransformPtr mTransform;
private:
	IRenderSystem& mRenderSys;
	ITexturePtr mTexture = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
private:
	SpriteVertexQuad mQuad;
	bool mQuadDirty;
	Eigen::Vector2f mPosition;
	Eigen::Vector2f mSize;
	bool mFlipY;
};

}