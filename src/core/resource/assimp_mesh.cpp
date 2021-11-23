#include "core/resource/assimp_mesh.h"
#include "core/resource/resource_manager.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** TMesh **********/
AssimpMesh::AssimpMesh(Launch launchMode, ResourceManager& resourceMng,
	const aiMesh* data,
	std::vector<AssimpMeshVertex>& vertices,
	std::vector<UINT>& indices,
	TextureBySlotPtr textures,
	MaterialPtr material)
{
	mData = data;
	mVertices.swap(vertices); vertices.clear();
	mIndices.swap(indices); indices.clear();
	mTextures = textures;
	mMaterial = material;

	mVertexBuffer = resourceMng.CreateVertexBuffer(launchMode, sizeof(AssimpMeshVertex) * mVertices.size(), sizeof(AssimpMeshVertex), 0, &mVertices[0]);
	mIndexBuffer = resourceMng.CreateIndexBuffer(launchMode, sizeof(UINT) * mIndices.size(), kFormatR32UInt, &mIndices[0]);
}

bool AssimpMesh::HasTexture(int slot) const
{
	return (slot < mTextures->Count()) 
		&& mTextures->At(slot)
		&& mTextures->At(slot)->HasSRV();
}

int AssimpMesh::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded()
		|| !mVertexBuffer->IsLoaded()
		|| !mIndexBuffer->IsLoaded()
		|| !mTextures->IsLoaded())
		return 0;

	RenderOperation op = {};
	op.mMaterial = mMaterial;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	op.mTextures = *mTextures;
	opList.AddOP(op);
	return 1;
}

}