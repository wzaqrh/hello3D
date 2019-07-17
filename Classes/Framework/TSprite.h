#pragma once
#include "TRenderSystem.h"

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
};

class TSprite
{
private:
	Quad mQuad;
	XMFLOAT2 mPosition;
	XMFLOAT2 mSize;
	TMaterialPtr mMaterial;
	ID3D11ShaderResourceView* mTexture = nullptr;
	ID3D11Buffer *mVertexBuffer = nullptr, *mIndexBuffer = nullptr;
	TRenderSystem* mRenderSys = nullptr;
public:
	TSprite(TRenderSystem* RenderSys, const char* vsName, const char* psName);
	~TSprite();
	void SetPosition(float x, float y);
	void SetSize(float w, float h);
	void SetTexture(ID3D11ShaderResourceView* Texture);

	void Draw();
};
typedef std::shared_ptr<TSprite> TSpritePtr;

