#include "core/renderable/renderable_base.h"
#include "core/renderable/paint3d.h"
#include "core/base/debug.h"
#include "core/scene/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace rend {

RenderableSingleRenderOp::RenderableSingleRenderOp(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& matInst)
	: mResMng(resourceMng)
	, mLaunchMode(launchMode)
	, mMaterial(matInst)
{}

const ITexturePtr& RenderableSingleRenderOp::GetTexture() const 
{
	return mMaterial->GetTextures()[0];
}

Eigen::AlignedBox3f RenderableSingleRenderOp::GetWorldAABB() const
{
	Transform3fAffine t(GetTransform()->GetWorldMatrix());
	Eigen::AlignedBox3f worldAABB = mAABB.transformed(t);
	return worldAABB;
}

CoTask<void> RenderableSingleRenderOp::UpdateFrame(float dt)
{
#if MIR_MATERIAL_HOTLOAD
	if (mMaterial && mMaterial->IsOutOfDate()) {
		mMaterial = CoAwait mResMng.CreateMaterialT(__LaunchAsync__, mMaterial->GetLoadParam());
	}
#endif
	CoReturn;
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

RenderOperation* RenderableSingleRenderOp::MakeRenderOperation(RenderOperationQueue& ops)
{
	if (!IsLoaded()) return nullptr;

#if MIR_GRAPHICS_DEBUG
	if (mDebugPaint) {
		mDebugPaint->Clear();
		mDebugPaint->DrawAABBEdge(GetWorldAABB());
		mDebugPaint->GenRenderOperation(ops);
	}
#endif

	RenderOperation op = {};
	op.Material = mMaterial;
	op.IndexBuffer = mIndexBuffer;
	op.AddVertexBuffer(mVertexBuffer);
	if (GetTransform())
		op.WorldTransform = GetTransform()->GetWorldMatrix();
	op.CameraMask = mCameraMask;
	op.CastShadow = mCastShadow;
	ops.AddOP(op);
	
	return &ops.Back();
}

}
}