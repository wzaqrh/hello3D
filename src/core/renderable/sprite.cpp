#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** Quad **********/
SpriteVertexQuad::SpriteVertexQuad()
{
	DoSetTexCoords(XMFLOAT2(0, 0), XMFLOAT2(1, 1));
	SetColor(XMFLOAT4(1, 1, 1, 1));
}

SpriteVertexQuad::SpriteVertexQuad(float x, float y, float w, float h)
{
	SetRect(x, y, w, h);
	DoSetTexCoords(XMFLOAT2(0, 0), XMFLOAT2(1, 1));
	SetColor(XMFLOAT4(1, 1, 1, 1));
}

void SpriteVertexQuad::SetRect(float x, float y, float w, float h)
{
	lb.Pos = XMFLOAT3(x, y, 0);
	lt.Pos = XMFLOAT3(x, y + h, 0);
	rt.Pos = XMFLOAT3(x + w, y + h, 0);
	rb.Pos = XMFLOAT3(x + w, y, 0);
}

void SpriteVertexQuad::SetColor(const XMFLOAT4& color)
{
	unsigned char c[4] = {
		static_cast<unsigned char>(color.x * 255),
		static_cast<unsigned char>(color.y * 255),
		static_cast<unsigned char>(color.z * 255),
		static_cast<unsigned char>(color.w * 255),
	};
	lb.Color = *((int*)c);
	lt.Color = *((int*)c);
	rt.Color = *((int*)c);
	rb.Color = *((int*)c);
}

void SpriteVertexQuad::SetZ(float z)
{
	lb.Pos.z = z;
	lt.Pos.z = z;
	rt.Pos.z = z;
	rb.Pos.z = z;
}

void SpriteVertexQuad::FlipY()
{
	std::swap(lb.Tex.y, rt.Tex.y);
	DoSetTexCoords(lb.Tex, rt.Tex);
}

void SpriteVertexQuad::DoSetTexCoords(XMFLOAT2 plb, XMFLOAT2 prt)
{
	lb.Tex = XMFLOAT2(plb.x, plb.y);
	lt.Tex = XMFLOAT2(plb.x, prt.y);
	rt.Tex = XMFLOAT2(prt.x, prt.y);
	rb.Tex = XMFLOAT2(prt.x, plb.y);
}

void SpriteVertexQuad::SetTexCoord(const XMFLOAT2& uv0, const XMFLOAT2& uv1)
{
	DoSetTexCoords(uv0, uv1);
}



/********** TSprite **********/
const unsigned int indices[] = {
	0, 1, 2, 0, 2, 3 
};
Sprite::Sprite(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& matName)
	: mRenderSys(renderSys)
	, mQuad(0, 0, 0, 0)
	, mQuadDirty(true)
	, mFlipY(true)
	, mSize(0, 0)
	, mPosition(0, 0)
{
	Transform = std::make_shared<Movable>();
	Material = matFac.GetMaterial(matName != "" ? matName : E_MAT_SPRITE);
	mIndexBuffer = mRenderSys.CreateIndexBuffer(sizeof(indices), DXGI_FORMAT_R32_UINT, (void*)&indices[0]);
	mVertexBuffer = mRenderSys.CreateVertexBuffer(sizeof(SpriteVertexQuad), sizeof(SpriteVertex), 0);

	if (mFlipY) mQuad.FlipY();
}

Sprite::~Sprite()
{
}

void Sprite::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT2(x, y);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
	mQuad.SetZ(z);
	//mQuad.FlipY(mFlipY);
	
	mQuadDirty = true;
	//mRenderSys.UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void Sprite::SetSize(float w, float h)
{
	if (mTexture != nullptr) {
		if (w == 0) w = mTexture->GetWidth();
		if (h == 0) h = mTexture->GetHeight();
	}
	mSize = XMFLOAT2(w, h);
	mQuad.SetRect(mPosition.x, mPosition.y, mSize.x, mSize.y);
	//mQuad.FlipY(mFlipY);

	mQuadDirty = true;
	//mRenderSys.UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void Sprite::SetSize(const XMFLOAT2& size)
{
	SetSize(size.x, size.y);
}

void Sprite::SetFlipY(bool flipY)
{
	if (mFlipY != flipY) {
		mFlipY = flipY;
		mQuad.FlipY();
	}

	mQuadDirty = true;
	//mRenderSys.UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void Sprite::SetColor(XMFLOAT4 color)
{
	mQuad.SetColor(color);

	mQuadDirty = true;
	//mRenderSys.UpdateBuffer(mVertexBuffer.Get(), &mQuad, sizeof(mQuad));
}

void Sprite::SetTexture(ITexturePtr Texture)
{
	mTexture = Texture;
	if (Texture != nullptr && (mSize.x == 0 || mSize.y == 0)) {
		SetSize(mSize.x, mSize.y == 0);
	}
}

void Sprite::Draw()
{
	mRenderSys.Draw(this);
}

int Sprite::GenRenderOperation(RenderOperationQueue& opList)
{
	if (mQuadDirty) {
		mQuadDirty = false;
		mRenderSys.UpdateBuffer((mVertexBuffer), &mQuad, sizeof(mQuad));
	}

	RenderOperation op = {};
	op.mMaterial = Material;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	if (mTexture) op.mTextures.Add(mTexture);
	op.mWorldTransform = Transform->Matrix();
	opList.AddOP(op);
	return 1;
}

}