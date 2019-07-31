#pragma once
#include "TRenderSystem.h"
#include "TMovable.h"

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
	ID3D11ShaderResourceView* mTexture = nullptr;
	TVertexBufferPtr mVertexBuffer;
	TIndexBufferPtr mIndexBuffer;
	TRenderSystem* mRenderSys = nullptr;
public:
	TMaterialPtr mMaterial;
	TMovablePtr mMove;
public:
	TSprite(TRenderSystem* RenderSys, const char* vsName, const char* psName);
	~TSprite();
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
public:
	void SetPosition(float x, float y, float z);
	void SetSize(float w, float h);
	void SetTexture(ID3D11ShaderResourceView* Texture);
	void SetFlipY(bool flipY);

	void Draw();
};
typedef std::shared_ptr<TSprite> TSpritePtr;

