#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material.h"

namespace mir {

/********** TMesh **********/
AssimpMesh::AssimpMesh(Launch launchMode, ResourceManager& resourceMng, const aiMesh* data,
	std::vector<AssimpMeshVertex>&& vertices, std::vector<UINT>&& indices, TextureBySlotPtr textures)
	: mAiMesh(data)
	, mVertices(std::move(vertices))
	, mIndices(std::move(indices))
	, mTextures(textures)
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt,  Data::Make(mIndices));
	mVertexBuffer = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(AssimpMeshVertex), 0, Data::Make(mVertices));
}

bool AssimpMesh::HasTexture(int slot) const
{
	return (slot < mTextures->Count()) 
		&& mTextures->At(slot)
		&& mTextures->At(slot)->HasSRV();
}

bool AssimpMesh::IsLoaded() const
{
	return (mVertexBuffer->IsLoaded()
		&& mIndexBuffer->IsLoaded()
		&& mTextures->IsLoaded());
}

}