#include "core/renderable/assimp_mesh.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** TMesh **********/
AssimpMesh::AssimpMesh(const aiMesh* __data,
	std::vector<AssimpMeshVertex>& __vertices,
	std::vector<UINT>& __indices,
	TTextureBySlotPtr __textures,
	MaterialPtr __material,
	IRenderSystem *__renderSys)
{
	Data = __data;
	Vertices.swap(__vertices); __vertices.clear();
	Indices.swap(__indices); __indices.clear();
	Textures = __textures;
	Material = __material;

	setupMesh(__renderSys);
}

bool AssimpMesh::setupMesh(IRenderSystem *renderSys)
{
	VertexBuffer = renderSys->CreateVertexBuffer(sizeof(AssimpMeshVertex) * Vertices.size(), sizeof(AssimpMeshVertex), 0, &Vertices[0]);
	IndexBuffer = renderSys->CreateIndexBuffer(sizeof(UINT) * Indices.size(), DXGI_FORMAT_R32_UINT, &Indices[0]);
	return true;
}

bool AssimpMesh::HasTexture(int slot)
{
	return (slot < Textures->Count()) 
		&& Textures->At(slot)
		&& Textures->At(slot)->HasSRV();
}

int AssimpMesh::GenRenderOperation(RenderOperationQueue& opList)
{
	RenderOperation op = {};
	op.mMaterial = Material;
	op.mIndexBuffer = IndexBuffer;
	op.mVertexBuffer = VertexBuffer;
	op.mTextures = *Textures;
	opList.AddOP(op);
	return 1;
}

}