#include "core/renderable/renderable_base.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace renderable {

/********** TSprite **********/
RenderableSingleRenderOp::RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng, const ShaderLoadParam& matName)
	: mResourceMng(resourceMng)
	, mLaunchMode(launchMode)
	, mCameraMask(-1)
{
	mTransform = CreateInstance<Transform>();
	mMaterial = resourceMng.CreateShader(launchMode, matName);
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
	op.Shader = mMaterial;
	op.IndexBuffer = mIndexBuffer;
	op.AddVertexBuffer(mVertexBuffer);
	if (mTexture) op.Textures.Add(mTexture);
	op.WorldTransform = mTransform->GetSRT();
	op.CameraMask = mCameraMask;
	return true;
}

}
}