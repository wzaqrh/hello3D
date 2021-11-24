#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_factory.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** TMesh **********/
AssimpMesh::AssimpMesh(Launch launchMode, ResourceManager& resourceMng, const aiMesh* data,
	std::vector<AssimpMeshVertex>&& vertices, std::vector<UINT>&& indices, TextureBySlotPtr textures)
	: mAiMesh(data)
	, mVertices(std::move(vertices))
	, mIndices(std::move(indices))
	, mTextures(textures)
{
	mVertexBuffer = resourceMng.CreateVertexBuffer(launchMode, sizeof(AssimpMeshVertex) * mVertices.size(), sizeof(AssimpMeshVertex), 0, &mVertices[0]);
	mIndexBuffer = resourceMng.CreateIndexBuffer(launchMode, sizeof(UINT) * mIndices.size(), kFormatR32UInt, &mIndices[0]);
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