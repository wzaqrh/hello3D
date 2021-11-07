#pragma once
#include "core/renderable/renderable.h"
#include "core/rendersys/interface_type_pred.h"

namespace mir {

struct Pos3Color3Tex2 {
	XMFLOAT3 Pos;
	unsigned int Color;
	XMFLOAT2 Tex;
};

struct Quad {
#if _MSC_VER <= 1800
	Pos3Color3Tex2 lb, lt, rt, rb;
#else
	union {
		Pos3Color3Tex2 m[4];
		struct { Pos3Color3Tex2 lb,lt,rt,rb; };
	};
#endif
	Quad();
	Quad(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetColor(const XMFLOAT4& color);
	void SetZ(float z);
	void FlipY();
	void SetTexCoord(const XMFLOAT2& uv0, const XMFLOAT2& uv1);
private:
	void DoSetTexCoords(XMFLOAT2 plb, XMFLOAT2 prt);
};

class Sprite : public IRenderable {
private:
	Quad mQuad;
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
	void Draw();
public:
	void SetPosition(float x, float y, float z);
	void SetSize(const XMFLOAT2& size);
	void SetSize(float w, float h);
	void SetTexture(ITexturePtr Texture);
	void SetColor(XMFLOAT4 color);
	void SetFlipY(bool flipY);
	const Quad* GetQuad() { return &mQuad; }
};
typedef std::shared_ptr<Sprite> SpritePtr;

}