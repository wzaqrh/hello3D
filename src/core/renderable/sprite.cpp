#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** TSprite **********/
Sprite::Sprite(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName)
	: Super(launchMode, resourceMng, matName)
	, mQuad(Eigen::Vector2f::Zero(), Eigen::Vector2f::Zero())
	, mQuadDirty(true)
	, mColor(Eigen::Vector4f::Ones())
	, mSize(Eigen::Vector3f::Zero())
	, mPosition(Eigen::Vector3f::Zero())
	, mAnchor(Eigen::Vector3f::Zero())
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(vbSurfaceQuad::GetIndices()));
	mVertexBuffer = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::MakeSize(sizeof(vbSurfaceQuad)));

	mQuad.FlipY();
}

void Sprite::SetPosition(const Eigen::Vector3f& pos)
{
	mPosition = pos;
	mQuadDirty = true;
}

//position = origin + size * anchor
void Sprite::SetAnchor(const Eigen::Vector3f& anchor)
{
	mAnchor = anchor;
	mQuadDirty = true;
}

void Sprite::SetSize(const Eigen::Vector3f& sz)
{
	Eigen::Vector3f size = sz;
	if (mTexture && !size.any()) {
		size.x() = mTexture->GetWidth();
		size.y() = mTexture->GetHeight();
	}

	if (mSize != size) {
		mSize = size;
		mQuadDirty = true;
	}
}

#if 0
void Sprite::SetFlipY(bool flipY)
{
	if (mFlipY != flipY) {
		mFlipY = flipY;
		mQuad.FlipY();
		mQuadDirty = true;
	}
}
#endif

void Sprite::SetColor(const Eigen::Vector4f& color)
{
	mColor = color;
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

		Eigen::Vector3f origin = mPosition - mAnchor.cwiseProduct(mSize);
		mQuad.SetCornerByRect(origin.head<2>(), mSize.head<2>());
		mQuad.SetZ(mPosition.z());
		mQuad.SetColor(mColor);

		mResourceMng.UpdateBuffer(mVertexBuffer, Data::Make(mQuad));
	}
	op.WorldTransform = mTransform->GetSRT();
	opList.AddOP(op);
}

}