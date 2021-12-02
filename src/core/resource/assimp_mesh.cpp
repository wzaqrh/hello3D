#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"

namespace mir {

/********** TMesh **********/
AssimpMesh::AssimpMesh(Launch launchMode, ResourceManager& resourceMng, const aiMesh* data,
	std::vector<vbSurface>&& surfVertexs, std::vector<vbSkeleton>&& skeletonVertexs, 
	std::vector<UINT>&& indices, TextureBySlotPtr textures)
	: mAiMesh(data)
	, mSurfVertexs(std::move(surfVertexs))
	, mSkeletonVertexs(std::move(skeletonVertexs))
	, mIndices(std::move(indices))
	, mTextures(textures)
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt,  Data::Make(mIndices));
	mVBOSurface = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::Make(mSurfVertexs));
	mVBOSkeleton = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSkeleton), 0, Data::Make(mSkeletonVertexs));
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