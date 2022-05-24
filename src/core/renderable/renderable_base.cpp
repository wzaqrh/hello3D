#include "core/renderable/renderable_base.h"
#include "core/base/debug.h"
#include "core/scene/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace rend {

RenderableSingleRenderOp::RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& matInst)
	: mResourceMng(resourceMng)
	, mLaunchMode(launchMode)
	, mMaterial(matInst)
{}

const ITexturePtr& RenderableSingleRenderOp::GetTexture() const 
{
	return mMaterial->GetTextures()[0];
}

Eigen::AlignedBox3f RenderableSingleRenderOp::GetWorldAABB() const
{
	Eigen::AlignedBox3f bounds = mAABB;
	if (!mAABB.isEmpty()) {
		if (auto transform = this->GetTransform()) {
			Transform3fAffine t(transform->GetWorldMatrix());
			bounds = mAABB.transformed(t);
		}
	}
	return bounds;
}

void RenderableSingleRenderOp::SetTexture(const ITexturePtr& texture)
{
	mMaterial->GetTextures()[0] = texture;
}

bool RenderableSingleRenderOp::IsLoaded() const
{
	BOOST_ASSERT(mMaterial);
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
	if (GetTransform())
		op.WorldTransform = GetTransform()->GetWorldMatrix();
	op.CameraMask = mCameraMask;
	return true;
}

}
}