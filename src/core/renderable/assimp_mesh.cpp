#include "core/renderable/assimp_mesh.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** TMesh **********/
AssimpMesh::AssimpMesh(const aiMesh* data,
	std::vector<AssimpMeshVertex>& vertices,
	std::vector<UINT>& indices,
	TTextureBySlotPtr textures,
	MaterialPtr material,
	IRenderSystem& renderSys)
{
	Data = data;
	Vertices.swap(vertices); vertices.clear();
	Indices.swap(indices); indices.clear();
	Textures = textures;
	Material = material;

	setupMesh(renderSys);
}

bool AssimpMesh::setupMesh(IRenderSystem& renderSys)
{
	VertexBuffer = renderSys.CreateVertexBuffer(sizeof(AssimpMeshVertex) * Vertices.size(), sizeof(AssimpMeshVertex), 0, &Vertices[0]);
	IndexBuffer = renderSys.CreateIndexBuffer(sizeof(UINT) * Indices.size(), DXGI_FORMAT_R32_UINT, &Indices[0]);
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