#include "core/renderable/sprite.h"
#include "core/scene/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace rend {

/********** TSprite **********/
Sprite::Sprite(Launch lchMode, ResourceManager& resMng, const res::MaterialInstance& material)
	: Super(lchMode, resMng, material)
	, mQuad(Eigen::Vector2f::Zero(), Eigen::Vector2f::Zero())
	, mQuadDirty(true)
	, mColor(Eigen::Vector4f::Ones())
	, mSize(Eigen::Vector3f::Zero())
	, mPosition(Eigen::Vector3f::Zero())
	, mAnchor(Eigen::Vector3f::Zero())
{
	mIndexBuffer = resMng.CreateIndexBuffer(lchMode, mVao, kFormatR32UInt, Data::Make(vbSurfaceQuad::GetIndices()));
	mVertexBuffer = resMng.CreateVertexBuffer(lchMode, mVao, sizeof(vbSurface), 0, Data::MakeSize(sizeof(vbSurfaceQuad)));

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

	if (GetTexture() && !size.any()) {
		size.x() = GetTexture()->GetWidth();
		size.y() = GetTexture()->GetHeight();
	}

	if (mSize != size) {
		mSize = size;
		mQuadDirty = true;
	}
}

void Sprite::SetColor(const Eigen::Vector4f& color)
{
	mColor = color;
	mQuadDirty = true;
}

void Sprite::SetTexture(const ITexturePtr& texture)
{
	BOOST_ASSERT(texture == nullptr || texture->IsLoaded());
	Super::SetTexture(texture);
	SetSize(mSize);
}

void Sprite::GenRenderOperation(RenderOperationQueue& ops)
{
	if (auto op = MakeRenderOperation(ops)) {
		if (mQuadDirty) {
			mQuadDirty = false;

			Eigen::Vector3f origin = mPosition - mAnchor.cwiseProduct(mSize);
			mQuad.SetCornerByRect(origin.head<2>(), mSize.head<2>());
			mQuad.SetZ(mPosition.z());
			mQuad.SetColor(mColor);

			mResMng.UpdateBuffer(mVertexBuffer, Data::Make(mQuad));
		}
		if (GetTransform()) op->WorldTransform = GetTransform()->GetWorldMatrix();
	}
}

}
}