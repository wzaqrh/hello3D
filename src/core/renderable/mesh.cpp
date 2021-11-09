#include "core/renderable/mesh.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** TMesh **********/
Mesh::Mesh(IRenderSystem& renderSys, MaterialFactory& matFac, const std::string& matName, int vertCount, int indexCount)
	:mRenderSys(renderSys)
{
	Material = matFac.GetMaterial(matName != "" ? matName : E_MAT_SPRITE);

	Vertices.resize(vertCount);
	VertexBuffer = mRenderSys.CreateVertexBuffer(sizeof(MeshVertex) * vertCount, sizeof(MeshVertex), 0, nullptr);
	
	Indices.resize(indexCount);
	IndexBuffer = mRenderSys.CreateIndexBuffer(sizeof(UINT) * indexCount, DXGI_FORMAT_R32_UINT, &Indices[0]);
	
	SubMeshs.resize(1);
}

int Mesh::GenRenderOperation(RenderOperationQueue& opList)
{
	if (VertDirty)
	{
		VertDirty = false;
		mRenderSys.UpdateBuffer(VertexBuffer, &Vertices[0], Vertices.size() * sizeof(MeshVertex));
	}

	if (IndiceDirty)
	{
		IndiceDirty = false;
		mRenderSys.UpdateBuffer(IndexBuffer, &Indices[0], Indices.size() * sizeof(UINT));
	}

	int opCount = 0;
	for (int i = 0; i < SubMeshs.size(); ++i)
	if (SubMeshs[i].IndiceCount > 0)
	{
		RenderOperation op = {};
		op.mMaterial = Material;
		op.mIndexBuffer = IndexBuffer;
		op.mVertexBuffer = VertexBuffer;
		op.mTextures = SubMeshs[i].Textures;
		op.mIndexPos = SubMeshs[i].IndicePos;
		op.mIndexCount = SubMeshs[i].IndiceCount;
		op.mIndexBase = SubMeshs[i].IndiceBase;
		opList.AddOP(op);
		++opCount;
	}
	return opCount;
}

void Mesh::Clear()
{
	VertDirty = true;
	VertPos = 0;

	IndiceDirty = true;
	SubMeshs.resize(1);
	SubMeshs[0].IndiceCount = 0;
	SubMeshs[0].IndicePos = 0;
}

void Mesh::SetVertexs(const MeshVertex* vertData, int vertCount)
{
	VertDirty = true;
	VertPos = vertCount;
	Vertices.assign(vertData, vertData + vertCount);
}

void Mesh::SetVertexs(const MeshVertex* vertData, int vertCount, int vertPos)
{
	VertDirty = true;
	VertPos = max(VertPos, vertPos + vertCount);
	for (int i = 0; i < vertCount; ++i)
		Vertices[i + vertPos] = vertData[i];
}

void Mesh::SetPositions(const XMFLOAT3* posData, int count)
{
	VertDirty = true;
	VertPos = max(VertPos, count);
	for (int i = 0; i < VertPos; ++i)
		Vertices[i].Position = posData[i];
}

void Mesh::SetColors(const XMFLOAT4* colorData, int count)
{
	VertDirty = true;
	VertPos = max(VertPos, count);
	for (int i = 0; i < VertPos; ++i) {
		unsigned char c[4] = {
			static_cast<unsigned char>(colorData[i].x * 255),
			static_cast<unsigned char>(colorData[i].y * 255),
			static_cast<unsigned char>(colorData[i].z * 255),
			static_cast<unsigned char>(colorData[i].w * 255),
		};
		Vertices[i].Color = *((int*)c);
	}
}

void Mesh::SetUVs(const XMFLOAT2* uvData, int count)
{
	VertDirty = true;
	VertPos = max(VertPos, count);
	for (int i = 0; i < VertPos; ++i)
		Vertices[i].UV = uvData[i];
}

void Mesh::SetSubMeshCount(int count)
{
	SubMeshs.resize(count);
}

void Mesh::SetIndices(const UINT* indiceData, int indicePos, int indiceCount, int indiceBase, int subMeshIndex)
{
	assert(subMeshIndex < SubMeshs.size());
	IndiceDirty = true;
	for (int i = 0; i < indiceCount; ++i)
		Indices[i + indicePos] = indiceData[i];

	auto& submesh = SubMeshs[subMeshIndex];
	submesh.IndicePos = indicePos;
	submesh.IndiceCount = indiceCount;
	submesh.IndiceBase = indiceBase;
}

void Mesh::SetTexture(int slot, ITexturePtr texture, int subMeshIndex)
{
	assert(subMeshIndex < SubMeshs.size());
	auto& submesh = SubMeshs[subMeshIndex];
	submesh.Textures[slot] = texture;
}

}