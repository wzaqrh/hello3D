#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** TSprite **********/
Sprite::Sprite(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName)
	: Super(launchMode, resourceMng, matName)
	, mQuad(Eigen::Vector2f(0, 0), Eigen::Vector2f(0, 0))
	, mQuadDirty(true)
	, mFlipY(true)
	, mSize(0, 0)
	, mPosition(0, 0)
{
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

void Sprite::GenRenderOperation(RenderOperationQueue& opList)
{
	RenderOperation op = {};
	if (!MakeRenderOperation(op)) return;

	if (mQuadDirty) {
		mQuadDirty = false;
		mResourceMng.UpdateBuffer(mVertexBuffer, Data::Make(mQuad));
	}
	op.WorldTransform = mTransform->GetSRT();
	opList.AddOP(op);
}

}