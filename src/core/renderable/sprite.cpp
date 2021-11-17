#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** Quad **********/
SpriteVertexQuad::SpriteVertexQuad()
{
	DoSetTexCoords(Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1));
	SetColor(Eigen::Vector4f(1, 1, 1, 1));
}

SpriteVertexQuad::SpriteVertexQuad(float x, float y, float w, float h)
{
	SetRect(x, y, w, h);
	DoSetTexCoords(Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1));
	SetColor(Eigen::Vector4f(1, 1, 1, 1));
}

void SpriteVertexQuad::SetRect(float x, float y, float w, float h)
{
	lb.Pos = Eigen::Vector3f(x, y, 0);
	lt.Pos = Eigen::Vector3f(x, y + h, 0);
	rt.Pos = Eigen::Vector3f(x + w, y + h, 0);
	rb.Pos = Eigen::Vector3f(x + w, y, 0);
}

void SpriteVertexQuad::SetColor(const Eigen::Vector4f& color)
{
	unsigned char c[4] = {
		static_cast<unsigned char>(color.x() * 255),
		static_cast<unsigned char>(color.y() * 255),
		static_cast<unsigned char>(color.z() * 255),
		static_cast<unsigned char>(color.w() * 255),
	};
	lb.Color = *((int*)c);
	lt.Color = *((int*)c);
	rt.Color = *((int*)c);
	rb.Color = *((int*)c);
}

void SpriteVertexQuad::SetZ(float z)
{
	lb.Pos.z() = z;
	lt.Pos.z() = z;
	rt.Pos.z() = z;
	rb.Pos.z() = z;
}

void SpriteVertexQuad::FlipY()
{
	std::swap(lb.Tex.y(), rt.Tex.y());
	DoSetTexCoords(lb.Tex, rt.Tex);
}

void SpriteVertexQuad::DoSetTexCoords(Eigen::Vector2f plb, Eigen::Vector2f prt)
{
	lb.Tex = Eigen::Vector2f(plb.x(), plb.y());
	lt.Tex = Eigen::Vector2f(plb.x(), prt.y());
	rt.Tex = Eigen::Vector2f(prt.x(), prt.y());
	rb.Tex = Eigen::Vector2f(prt.x(), plb.y());
}

void SpriteVertexQuad::SetTexCoord(const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1)
{
	DoSetTexCoords(uv0, uv1);
}

/********** TSprite **********/
const unsigned int indices[] = {
	0, 1, 2, 0, 2, 3 
};
Sprite::Sprite(ResourceManager& resourceMng, MaterialFactory& matFac, const std::string& matName)
	: mResourceMng(resourceMng)
	, mQuad(0, 0, 0, 0)
	, mQuadDirty(true)
	, mFlipY(true)
	, mSize(0, 0)
	, mPosition(0, 0)
{
	mTransform = std::make_shared<Transform>();
	mMaterial = matFac.GetMaterial(matName != "" ? matName : E_MAT_SPRITE);
	mIndexBuffer = resourceMng.CreateIndexBuffer(sizeof(indices), kFormatR32UInt, (void*)&indices[0]);
	mVertexBuffer = resourceMng.CreateVertexBuffer(sizeof(SpriteVertexQuad), sizeof(SpriteVertex), 0, nullptr);

	if (mFlipY) mQuad.FlipY();
}

Sprite::~Sprite()
{
}

void Sprite::SetPosition(const Eigen::Vector3f& pos)
{
	mPosition = Eigen::Vector2f(pos.x(), pos.y());
	mQuad.SetRect(mPosition.x(), mPosition.y(), mSize.x(), mSize.y());
	mQuad.SetZ(pos.z());
	mQuadDirty = true;
}

void Sprite::SetSize(const Eigen::Vector2f& sz)
{
	Eigen::Vector2f size = sz;
	if (mTexture != nullptr) {
		if (size.x() == 0) size.x() = mTexture->GetWidth();
		if (size.y() == 0) size.y() = mTexture->GetHeight();
	}
	mSize = size;
	mQuad.SetRect(mPosition.x(), mPosition.y(), mSize.x(), mSize.y());
	mQuadDirty = true;
}

void Sprite::SetFlipY(bool flipY)
{
	if (mFlipY != flipY) {
		mFlipY = flipY;
		mQuad.FlipY();
		mQuadDirty = true;
	}
}

void Sprite::SetColor(const Eigen::Vector4f& color)
{
	mQuad.SetColor(color);
	mQuadDirty = true;
}

void Sprite::SetTexture(const ITexturePtr& texture)
{
	mTexture = texture;
	if (texture != nullptr && (mSize.x() == 0 || mSize.y() == 0)) {
		SetSize(Eigen::Vector2f());
	}
}

int Sprite::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded() || !mVertexBuffer->IsLoaded() || !mIndexBuffer->IsLoaded())
		return 0;

	if (mQuadDirty) {
		mQuadDirty = false;
		mResourceMng.UpdateBuffer(mVertexBuffer, &mQuad, sizeof(mQuad));
	}

	RenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	if (mTexture) op.mTextures.Add(mTexture);
	op.mWorldTransform = mTransform->GetMatrix();
	opList.AddOP(op);
	return 1;
}

}