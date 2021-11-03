#pragma once
#include "IRenderable.h"
#include "rendersys/TInterfaceTypePred.h"

struct Pos3Color3Tex2
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	XMFLOAT2 Tex;
};

struct Quad
{
#if _MSC_VER <= 1800
	Pos3Color3Tex2 lb, lt, rt, rb;
#else
	union {
		Pos3Color3Tex2 m[4];
		struct {
			Pos3Color3Tex2 lb,lt,rt,rb;
		};
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

class TSprite
	: public IRenderable
{
private:
	Quad mQuad;
	bool mQuadDirty;
	XMFLOAT2 mPosition;
	XMFLOAT2 mSize;
	bool mFlipY;
private:
	IRenderSystem* mRenderSys = nullptr;
	ITexturePtr mTexture = nullptr;
	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
public:
	TMaterialPtr Material;
	TTransformPtr Transform;
public:
	TSprite(IRenderSystem* RenderSys, const std::string& matName = "");
	~TSprite();
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
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
typedef std::shared_ptr<TSprite> TSpritePtr;

