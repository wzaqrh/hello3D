#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** SpriteVertexQuad **********/
SpriteVertexQuad::SpriteVertexQuad()
{
	DoSetTexCoords(Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1));
	SetColor(Eigen::Vector4f(1, 1, 1, 1));
}

SpriteVertexQuad::SpriteVertexQuad(const Eigen::Vector2f& origin, const Eigen::Vector2f& size)
{
	SetCornerByRect(origin, size);
	DoSetTexCoords(Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1));
	SetColor(Eigen::Vector4f(1, 1, 1, 1));
}

void SpriteVertexQuad::SetCornerByRect(const Eigen::Vector2f& org, const Eigen::Vector2f& size, float z)
{
	lb().Pos = Eigen::Vector3f(org.x(), org.y(), z);
	lt().Pos = Eigen::Vector3f(org.x(), org.y() + size.y(), z);
	rt().Pos = Eigen::Vector3f(org.x() + size.x(), org.y() + size.y(), z);
	rb().Pos = Eigen::Vector3f(org.x() + size.x(), org.y(), z);
}
void SpriteVertexQuad::SetCornerByLBRT(const Eigen::Vector2f& pLB, const Eigen::Vector2f& pRT, float z)
{
	SetCornerByRect(pLB, pRT - pLB, z);
}

void SpriteVertexQuad::SetCornerByVector(const Eigen::Vector3f& org, const Eigen::Vector3f& right, const Eigen::Vector3f& up)
{
	lb().Pos = org;
	lt().Pos = org + up;
	rt().Pos = org + up + right;
	rb().Pos = org + right;
}

void SpriteVertexQuad::SetColor(const Eigen::Vector4f& color)
{
	unsigned char c[4] = {
		static_cast<unsigned char>(color.x() * 255),
		static_cast<unsigned char>(color.y() * 255),
		static_cast<unsigned char>(color.z() * 255),
		static_cast<unsigned char>(color.w() * 255),
	};
	for (size_t i = 0; i < kCubeConerFrontCount; ++i)
		Coners[i].Color = *((int*)c);
}

void SpriteVertexQuad::SetZ(float z)
{
	for (size_t i = 0; i < kCubeConerFrontCount; ++i)
		Coners[i].Pos.z() = z;
}

void SpriteVertexQuad::FlipY()
{
	std::swap(lb().Tex.y(), rt().Tex.y());
	DoSetTexCoords(lb().Tex, rt().Tex);
}

void SpriteVertexQuad::DoSetTexCoords(Eigen::Vector2f plb, Eigen::Vector2f prt)
{
	lb().Tex = Eigen::Vector2f(plb.x(), plb.y());
	lt().Tex = Eigen::Vector2f(plb.x(), prt.y());
	rt().Tex = Eigen::Vector2f(prt.x(), prt.y());
	rb().Tex = Eigen::Vector2f(prt.x(), plb.y());
}

void SpriteVertexQuad::SetTexCoord(const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1)
{
	DoSetTexCoords(uv0, uv1);
}

/********** TSprite **********/
constexpr uint32_t CIndices[] = {
	0, 1, 2, 0, 2, 3 
};
Sprite::Sprite(Launch launchMode, ResourceManager& resourceMng, const std::string& matName)
	: mResourceMng(resourceMng)
	, mQuad(Eigen::Vector2f(0, 0), Eigen::Vector2f(0, 0))
	, mQuadDirty(true)
	, mFlipY(true)
	, mSize(1, 1)
	, mPosition(0, 0)
{
	mTransform = std::make_shared<Transform>();
	mMaterial = resourceMng.CreateMaterial(__launchMode__, matName != "" ? matName : E_MAT_SPRITE);
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(CIndices));
	mVertexBuffer = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(SpriteVertex), 0, Data::MakeSize(sizeof(SpriteVertexQuad)));

	if (mFlipY) mQuad.FlipY();
}

void Sprite::SetPosition(const Eigen::Vector3f& pos)
{
	mPosition = pos.head<2>();
	mQuad.SetCornerByRect(mPosition, mSize);
	mQuad.SetZ(pos.z());
	mQuadDirty = true;
}

void Sprite::SetSize(const Eigen::Vector2f& sz)
{
	Eigen::Vector2f size = sz;
	if (mTexture && !size.any()) {
		size.x() = mTexture->GetWidth();
		size.y() = mTexture->GetHeight();
	}

	if (mSize != size) {
		mSize = size;
		mQuad.SetCornerByRect(mPosition, mSize);
		mQuadDirty = true;
	}
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
	if (texture && !mSize.any()) {
		if (texture->IsLoaded()) {
			SetSize(mSize);
		}
		else {
			mResourceMng.AddResourceLoadedObserver(texture, [this](IResourcePtr res) {
				SetSize(mSize);	
			});
		}
	}
}

int Sprite::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded() 
		|| !mVertexBuffer->IsLoaded() 
		|| !mIndexBuffer->IsLoaded()
		|| (mTexture && !mTexture->IsLoaded()))
		return 0;

	if (mQuadDirty) {
		mQuadDirty = false;
		mResourceMng.UpdateBuffer(mVertexBuffer, Data::Make(mQuad));
	}

	RenderOperation op = {};
	op.Material = mMaterial;
	op.IndexBuffer = mIndexBuffer;
	op.VertexBuffer = mVertexBuffer;
	if (mTexture) op.Textures.Add(mTexture);
	op.WorldTransform = mTransform->GetMatrix();
	op.CameraMask = mCameraMask;
	opList.AddOP(op);
	return 1;
}

}