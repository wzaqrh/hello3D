#include "TSprite.h"
#include "TTransform.h"
#include "IRenderSystem.h"
#include "TMaterial.h"
#include "TInterfaceType.h"

/********** Quad **********/
Quad::Quad()
{
	DoSetTexCoords(XMFLOAT2(0, 0), XMFLOAT2(1, 1));
	SetColor(XMFLOAT4(1, 1, 1, 1));
}

Quad::Quad(float x, float y, float w, float h)
{
	SetRect(x, y, w, h);
	DoSetTexCoords(XMFLOAT2(0, 0), XMFLOAT2(1, 1));
	SetColor(XMFLOAT4(1, 1, 1, 1));
}

void Quad::SetRect(float x, float y, float w, float h)
{
	lb.Pos = XMFLOAT3(x, y, 0);
	lt.Pos = XMFLOAT3(x, y + h, 0);
	rt.Pos = XMFLOAT3(x + w, y + h, 0);
	rb.Pos = XMFLOAT3(x + w, y, 0);
}

void Quad::SetColor(const XMFLOAT4& color)
{
	lb.Color = color;
	lt.Color = color;
	rt.Color = color;
	rb.Color = color;
}

void Quad::SetZ(float z)
{
	lb.Pos.z = z;
	lt.Pos.z = z;
	rt.Pos.z = z;
	rb.Pos.z = z;
}

void Quad::FlipY()
{
	std::swap(lb.Tex.y, rt.Tex.y);
	DoSetTexCoords(lb.Tex, rt.Tex);
}

void Quad::DoSetTexCoords(XMFLOAT2 plb, XMFLOAT2 prt)
{
	lb.Tex = XMFLOAT2(plb.x, plb.y);
	lt.Tex = XMFLOAT2(plb.x, prt.y);
	rt.Tex = XMFLOAT2(prt.x, prt.y);
	rb.Tex = XMFLOAT2(prt.x, plb.y);
}

void Quad::SetTexCoord(const XMFLOAT2& uv0, const XMFLOAT2& uv1)
{
	DoSetTexCoords(uv0, uv1);
}



/********** TSprite **********/
const unsigned int indices[] = {
	0, 1, 2, 0, 2, 3 
};
TSprite::TSprite(IRenderSystem* RenderSys, const std::string& matName)
	: mQuad(0, 0, 0, 0)
	, mQuadDirty(true)
	, mFlipY(true)
	, mSize(0, 0)
	, mPosition(0, 0)
{
	Transform = std::make_shared<TMovable>();
	mRenderSys = RenderSys;
	Material = mRenderSys->CreateMaterial(matName != "" ? matName : E_MAT_SPRITE, nullptr);
	mIndexBuffer = mRenderSys->CreateIndexBuffer(sizeof(indices), DXGI_FORMAT_R32_UINT, (void*)&indices[0]);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(Quad), sizeof(Pos3Color3Tex2), 0);

	if (mFlipY) mQuad.FlipY();
}

TSprite::~TSprite()
{
}

void TSprite::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT2(x, y);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
	mQuad.SetZ(z);
	//mQuad.FlipY(mFlipY);
	
	mQuadDirty = true;
	//mRenderSys->UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void TSprite::SetSize(float w, float h)
{
	if (mTexture != nullptr) {
		if (w == 0) w = mTexture->GetWidth();
		if (h == 0) h = mTexture->GetHeight();
	}
	mSize = XMFLOAT2(w, h);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
	//mQuad.FlipY(mFlipY);

	mQuadDirty = true;
	//mRenderSys->UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void TSprite::SetSize(const XMFLOAT2& size)
{
	SetSize(size.x, size.y);
}

void TSprite::SetFlipY(bool flipY)
{
	if (mFlipY != flipY) {
		mFlipY = flipY;
		mQuad.FlipY();
	}

	mQuadDirty = true;
	//mRenderSys->UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void TSprite::SetColor(XMFLOAT4 color)
{
	mQuad.SetColor(color);

	mQuadDirty = true;
	//mRenderSys->UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void TSprite::SetTexture(ITexturePtr Texture)
{
	mTexture = Texture;
	if (Texture != nullptr && (mSize.x == 0 || mSize.y == 0)) {
		SetSize(mSize.x, mSize.y == 0);
	}
}

void TSprite::Draw()
{
	mRenderSys->Draw(this);
}

int TSprite::GenRenderOperation(TRenderOperationQueue& opList)
{
	if (mQuadDirty) {
		mQuadDirty = false;
		mRenderSys->UpdateBuffer((mVertexBuffer), &mQuad, sizeof(mQuad));
	}

	TRenderOperation op = {};
	op.mMaterial = Material;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	if (mTexture) op.mTextures.push_back(mTexture);
	op.mWorldTransform = Transform->Matrix();
	opList.AddOP(op);
	return 1;
}