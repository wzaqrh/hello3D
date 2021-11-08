#pragma once
#include "core/rendersys/predeclare.h"
#include "core/renderable/renderable.h"

namespace mir {

struct SpriteVertex {
	XMFLOAT3 Pos;
	unsigned int Color;
	XMFLOAT2 Tex;
};

struct SpriteVertexQuad {
#if _MSC_VER <= 1800
	SpriteVertex lb, lt, rt, rb;
#else
	union {
		SpriteVertex m[4];
		struct { SpriteVertex lb,lt,rt,rb; };
	};
#endif
	SpriteVertexQuad();
	SpriteVertexQuad(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetColor(const XMFLOAT4& color);
	void SetZ(float z);
	void FlipY();
	void SetTexCoord(const XMFLOAT2& uv0, const XMFLOAT2& uv1);
private:
	void DoSetTexCoords(XMFLOAT2 plb, XMFLOAT2 prt);
};

class Sprite : public IRenderable 
{
private:
	SpriteVertexQuad mQuad;
	bool mQuadDirty;
	XMFLOAT2 mPosition;
	XMFLOAT2 mSize;
	bool mFlipY;
private:
	IRenderSystem& mRenderSys;
	ITexturePtr mTexture = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
public:
	MaterialPtr Material;
	TransformPtr Transform;
public:
	Sprite(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& matName = "");
	~Sprite();
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
public:
	void SetPosition(float x, float y, float z);
	void SetSize(const XMFLOAT2& size);
	void SetSize(float w, float h);
	void SetTexture(ITexturePtr Texture);
	void SetColor(XMFLOAT4 color);
	void SetFlipY(bool flipY);
	const SpriteVertexQuad* GetQuad() { return &mQuad; }
};

}