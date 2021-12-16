#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"
#include "core/base/debug.h"

namespace mir {

/********** TMesh **********/
AssimpMesh::AssimpMesh(Launch launchMode, ResourceManager& resourceMng, const aiMesh* data,
	std::vector<vbSurface, mir_allocator<vbSurface>>&& surfVertexs, 
	std::vector<vbSkeleton, mir_allocator<vbSkeleton>>&& skeletonVertexs, 
	std::vector<uint32_t>&& indices, TextureBySlotPtr textures)
	: mAiMesh(data)
	, mSurfVertexs(std::move(surfVertexs))
	, mSkeletonVertexs(std::move(skeletonVertexs))
	, mIndices(std::move(indices))
	, mTextures(textures)
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt,  Data::Make(mIndices));
	
	mVBOSurface = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::Make(mSurfVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.surface");

	mVBOSkeleton = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSkeleton), 0, Data::Make(mSkeletonVertexs));
	DEBUG_SET_PRIV_DATA(mVBOSurface, "assimp_mesh.skeleton");

#if defined _DEBUG
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

bool AssimpMesh::IsLoaded() const
{
	return (mVBOSurface->IsLoaded()
		&& mVBOSkeleton->IsLoaded()
		&& mIndexBuffer->IsLoaded()
		&& mTextures->IsLoaded());
}

}