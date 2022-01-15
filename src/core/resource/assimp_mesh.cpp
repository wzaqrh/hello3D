#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"
#include "core/base/debug.h"

namespace mir {
namespace res {

AssimpMesh::AssimpMesh()
{}

void AssimpMesh::Build(Launch launchMode, ResourceManager& resourceMng)
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(mIndices));
	DEBUG_SET_PRIV_DATA(mIndexBuffer, "assimp_mesh.index");

	mVBOSurface = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::Make(mSurfVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.surface");

	mVBOSkeleton = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSkeleton), 0, Data::Make(mSkeletonVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.skeleton");

#if defined _DEBUG && 0
	mMinPos = mMaxPos = Eigen::Vector3f::Zero();
	for (auto& it : mSurfVertexs) {
		mMinPos.x() = std::min<float>(mMinPos.x(), it.Pos.x());
		mMinPos.y() = std::min<float>(mMinPos.y(), it.Pos.y());
		mMinPos.z() = std::min<float>(mMinPos.z(), it.Pos.z());

		mMaxPos.x() = std::max<float>(mMaxPos.x(), it.Pos.x());
		mMaxPos.y() = std::max<float>(mMaxPos.y(), it.Pos.y());
		mMaxPos.z() = std::max<float>(mMaxPos.z(), it.Pos.z());
	}
#endif
}

bool AssimpMesh::HasTexture(int slot) const
{
	return (slot < mTextures.Count())
		&& mTextures[slot]
		&& mTextures[slot]->IsLoaded();
}

static Eigen::Vector4f sDefaultVector4;
const Eigen::Vector4f& AssimpMesh::GetFactor(int slot) const
{
	return slot < mFactors.size() ? mFactors[slot] : sDefaultVector4;
}

const Eigen::Vector4f& AssimpMesh::GetUvTransform(int slot) const
{
	return slot < mUvTransform.size() ? mUvTransform[slot] : sDefaultVector4;
}

bool AssimpMesh::IsLoaded() const
{
	return (mVBOSurface->IsLoaded()
		&& mVBOSkeleton->IsLoaded()
		&& mIndexBuffer->IsLoaded()
		&& mTextures.IsLoaded());
}

}
}