#include "TMesh.h"
#include "IRenderSystem.h"
#include "IRenderable.h"
#include "TInterfaceType.h"

/********** TMesh **********/
TMesh::TMesh(const aiMesh* __data,
	std::vector<MeshVertex>& __vertices,
	std::vector<UINT>& __indices,
	TTextureBySlotPtr __textures,
	TMaterialPtr __material,
	IRenderSystem *__renderSys)
{
	data = __data;
	vertices.swap(__vertices); __vertices.clear();
	indices.swap(__indices); __indices.clear();
	mTextures = __textures;
	mMaterial = __material;

	setupMesh(__renderSys);
}

bool TMesh::setupMesh(IRenderSystem *renderSys)
{
	mVertexBuffer = renderSys->CreateVertexBuffer(sizeof(MeshVertex) * vertices.size(), sizeof(MeshVertex), 0, &vertices[0]);
	mIndexBuffer = renderSys->CreateIndexBuffer(sizeof(UINT) * indices.size(), DXGI_FORMAT_R32_UINT, &indices[0]);
	return true;
}

void TMesh::Close()
{
}

bool TMesh::HasTexture(int slot)
{
	return (slot < mTextures->size()) 
		&& mTextures->At(slot)
		&& mTextures->At(slot)->HasSRV();
}