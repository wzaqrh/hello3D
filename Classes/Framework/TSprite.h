#pragma once
#include "IRenderable.h"
#include "TPredefine.h"

struct Pos3Color3Tex2
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	XMFLOAT2 Tex;
};

struct Quad
{
	union {
		Pos3Color3Tex2 m[4];
		struct {
			Pos3Color3Tex2 lb,lt,rt,rb;
		};
	};
	Quad(float x, float y, float w, float h);
	void SetRect(float x, float y, float w, float h);
	void SetColor(float r, float g, float b, float a);
	void SetZ(float z);
	void SetFlipY(bool flipY);
};

class TSprite
	: public IRenderable
{
private:
	Quad mQuad;
	XMFLOAT2 mPosition;
	XMFLOAT2 mSize;
private:
	IRenderSystem* mRenderSys = nullptr;

	TTexturePtr mTexture = nullptr;
	TVertexBufferPtr mVertexBuffer;
	TIndexBufferPtr mIndexBuffer;
public:
	TMaterialPtr mMaterial;
	TMovablePtr mMove;
public:
	TSprite(IRenderSystem* RenderSys, const char* vsName, const char* psName);
	~TSprite();
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
	void Draw();
public:
	void SetPosition(float x, float y, float z);
	void SetSize(float w, float h);
	void SetTexture(TTexturePtr Texture);
	void SetFlipY(bool flipY);
};
typedef std::shared_ptr<TSprite> TSpritePtr;

