#include "TAssimpMesh.h"
#include "IRenderSystem.h"
#include "IRenderable.h"
#include "TInterfaceType.h"

/********** TMesh **********/
TAssimpMesh::TAssimpMesh(const aiMesh* __data,
	std::vector<AssimpMeshVertex>& __vertices,
	std::vector<UINT>& __indices,
	TTextureBySlotPtr __textures,
	TMaterialPtr __material,
	IRenderSystem *__renderSys)
{
	Data = __data;
	Vertices.swap(__vertices); __vertices.clear();
	Indices.swap(__indices); __indices.clear();
	Textures = __textures;
	Material = __material;

	setupMesh(__renderSys);
}

bool TAssimpMesh::setupMesh(IRenderSystem *renderSys)
{
	VertexBuffer = renderSys->CreateVertexBuffer(sizeof(AssimpMeshVertex) * Vertices.size(), sizeof(AssimpMeshVertex), 0, &Vertices[0]);
	IndexBuffer = renderSys->CreateIndexBuffer(sizeof(UINT) * Indices.size(), DXGI_FORMAT_R32_UINT, &Indices[0]);
	return true;
}

bool TAssimpMesh::HasTexture(int slot)
{
	return (slot < Textures->size()) 
		&& Textures->At(slot)
		&& Textures->At(slot)->HasSRV();
}

int TAssimpMesh::GenRenderOperation(TRenderOperationQueue& opList)
{
	TRenderOperation op = {};
	op.mMaterial = Material;
	op.mIndexBuffer = IndexBuffer;
	op.mVertexBuffer = VertexBuffer;
	op.mTextures = *Textures;
	opList.AddOP(op);
	return 1;
}
