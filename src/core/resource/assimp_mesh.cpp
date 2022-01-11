#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"
#include "core/base/debug.h"

namespace mir {

AssimpMesh::AssimpMesh()
{
	mTextures = CreateInstance<TextureBySlot>();
}

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
	return (slot < mTextures->Count()) 
		&& mTextures->At(slot)
		&& mTextures->At(slot)->IsLoaded();
}

Eigen::Vector4f AssimpMesh::GetFactor(int slot) const
{
	return slot < mFactors.size() ? mFactors[slot] : Eigen::Vector4f::Zero();
}

bool AssimpMesh::IsLoaded() const
{
	return (mVBOSurface->IsLoaded()
		&& mVBOSkeleton->IsLoaded()
		&& mIndexBuffer->IsLoaded()
		&& mTextures->IsLoaded());
}

}