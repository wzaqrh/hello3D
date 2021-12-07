#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** TSprite **********/
Sprite::Sprite(Launch launchMode, ResourceManager& resourceMng, const std::string& matName)
	: mResourceMng(resourceMng)
	, mQuad(Eigen::Vector2f(0, 0), Eigen::Vector2f(0, 0))
	, mQuadDirty(true)
	, mFlipY(true)
	, mSize(0, 0)
	, mPosition(0, 0)
{
	mTransform = std::make_shared<Transform>();
	mMaterial = resourceMng.CreateMaterial(__launchMode__, matName != "" ? matName : E_MAT_SPRITE);
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(vbSurfaceQuad::GetIndices()));
	mVertexBuffer = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::MakeSize(sizeof(vbSurfaceQuad)));

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
	op.AddVertexBuffer(mVertexBuffer);
	if (mTexture) op.Textures.Add(mTexture);
	op.WorldTransform = mTransform->GetMatrix();
	op.CameraMask = mCameraMask;
	opList.AddOP(op);
	return 1;
}

}