#include "core/renderable/renderable_base.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {

/********** TSprite **********/
RenderableSingleRenderOp::RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName)
	: mResourceMng(resourceMng)
	, mLaunchMode(launchMode)
	, mCameraMask(-1)
{
	mTransform = std::make_shared<Transform>();
	mMaterial = resourceMng.CreateMaterial(launchMode, matName);
}

void RenderableSingleRenderOp::SetTexture(const ITexturePtr& texture)
{
	mTexture = texture;
}

bool RenderableSingleRenderOp::IsLoaded() const
{
	if (!mMaterial->IsLoaded()
		|| (mVertexBuffer && !mVertexBuffer->IsLoaded())
		|| (mIndexBuffer && !mIndexBuffer->IsLoaded())
		|| (mTexture && !mTexture->IsLoaded()))
		return false;
	return true;
}

bool RenderableSingleRenderOp::MakeRenderOperation(RenderOperation& op)
{
	if (!IsLoaded()) return false;

	op = RenderOperation{};
	op.Material = mMaterial;
	op.IndexBuffer = mIndexBuffer;
	op.AddVertexBuffer(mVertexBuffer);
	if (mTexture) op.Textures.Add(mTexture);
	op.WorldTransform = Eigen::Matrix4f::Identity();
	op.CameraMask = mCameraMask;
	return true;
}

}