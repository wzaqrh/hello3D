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
#if USE_MATERIAL_INSTANCE
	mMaterial = resourceMng.CreateMaterial(launchMode, matName.ShaderName);
#else
	mMaterial = resourceMng.CreateShader(launchMode, matName);
#endif
}

#if USE_MATERIAL_INSTANCE
const ITexturePtr& RenderableSingleRenderOp::GetTexture() const 
{
	return mMaterial->GetTextures()[0];
}
void RenderableSingleRenderOp::SetTexture(const ITexturePtr& texture)
{
	mMaterial->GetTextures()[0] = texture;
}
#else
void RenderableSingleRenderOp::SetTexture(const ITexturePtr& texture)
{
	mTexture = texture;
}
#endif

bool RenderableSingleRenderOp::IsLoaded() const
{
#if USE_MATERIAL_INSTANCE
	if (!mMaterial->IsLoaded()
		|| (mVertexBuffer && !mVertexBuffer->IsLoaded())
		|| (mIndexBuffer && !mIndexBuffer->IsLoaded()))
		return false;
#else
	if (!mMaterial->IsLoaded()
		|| (mVertexBuffer && !mVertexBuffer->IsLoaded())
		|| (mIndexBuffer && !mIndexBuffer->IsLoaded())
		|| (mTexture && !mTexture->IsLoaded()))
		return false;
#endif
	return true;
}

bool RenderableSingleRenderOp::MakeRenderOperation(RenderOperation& op)
{
	if (!IsLoaded()) return false;

	op = RenderOperation{};
#if USE_MATERIAL_INSTANCE
	op.Material = mMaterial;
#else
	op.Shader = mMaterial;
	if (mTexture) op.Textures.Add(mTexture);
#endif
	op.IndexBuffer = mIndexBuffer;
	op.AddVertexBuffer(mVertexBuffer);
	op.WorldTransform = mTransform->GetSRT();
	op.CameraMask = mCameraMask;
	return true;
}

}
}