#include "core/renderable/renderable_base.h"
#include "core/base/debug.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace rend {

RenderableSingleRenderOp::RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng)
	: mResourceMng(resourceMng)
	, mLaunchMode(launchMode)
	, mCameraMask(-1)
{}

CoTask<bool> RenderableSingleRenderOp::Init(const MaterialLoadParam& loadParam)
{
	COROUTINE_VARIABLES_1(loadParam);

	mTransform = CreateInstance<Transform>();
	mMaterial = CoAwait mResourceMng.CreateMaterial(mLaunchMode, loadParam);
	CoReturn true;
}

const ITexturePtr& RenderableSingleRenderOp::GetTexture() const 
{
	return mMaterial->GetTextures()[0];
}
void RenderableSingleRenderOp::SetTexture(const ITexturePtr& texture)
{
	mMaterial->GetTextures()[0] = texture;
}

bool RenderableSingleRenderOp::IsLoaded() const
{
	if (!mMaterial->IsLoaded()
		|| (mVertexBuffer && !mVertexBuffer->IsLoaded())
		|| (mIndexBuffer && !mIndexBuffer->IsLoaded()))
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
	op.WorldTransform = mTransform->GetSRT();
	op.CameraMask = mCameraMask;
	return true;
}

}
}