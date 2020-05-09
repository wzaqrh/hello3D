#include "TMesh.h"
#include "IRenderSystem.h"
#include "IRenderable.h"
#include "TMaterial.h"
#include "TInterfaceType.h"

/********** TMesh **********/
TMesh::TMesh(IRenderSystem* renderSys, const std::string& matName, int vertCount, int indexCount)
{
	mRenderSys = renderSys;
	Material = mRenderSys->CreateMaterial(matName != "" ? matName : E_MAT_SPRITE, nullptr);

	Vertices.resize(vertCount);
	VertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(MeshVertex) * vertCount, sizeof(MeshVertex), 0, nullptr);
	
	Indices.resize(indexCount);
	IndexBuffer = mRenderSys->CreateIndexBuffer(sizeof(UINT) * indexCount, DXGI_FORMAT_R32_UINT, &Indices[0]);
	
	SubMeshs.resize(1);
}

int TMesh::GenRenderOperation(TRenderOperationQueue& opList)
{
	if (VertDirty)
	{
		VertDirty = false;
		mRenderSys->UpdateBuffer(VertexBuffer, &Vertices[0], Vertices.size() * sizeof(MeshVertex));
	}

	if (IndiceDirty)
	{
		IndiceDirty = false;
		mRenderSys->UpdateBuffer(IndexBuffer, &Indices[0], Indices.size() * sizeof(UINT));
	}

	int opCount = 0;
	for (int i = 0; i < SubMeshs.size(); ++i)
	if (SubMeshs[i].IndiceCount > 0)
	{
		TRenderOperation op = {};
		op.mMaterial = Material;
		op.mIndexBuffer = IndexBuffer;
		op.mVertexBuffer = VertexBuffer;
		op.mTextures = SubMeshs[i].Textures;
		op.mIndexPos = SubMeshs[i].IndicePos;
		op.mIndexCount = SubMeshs[i].IndiceCount;
		opList.AddOP(op);
		++opCount;
	}
	return opCount;
}

void TMesh::Clear()
{
	VertDirty = true;
	VertPos = 0;

	IndiceDirty = true;
	SubMeshs.resize(1);
	SubMeshs[0].IndiceCount = 0;
	SubMeshs[0].IndicePos = 0;
}

void TMesh::SetVertexs(const MeshVertex* vertData, int vertCount)
{
	VertDirty = true;
	VertPos = vertCount;
	Vertices.assign(vertData, vertData + vertCount);
}

void TMesh::SetVertexs(const MeshVertex* vertData, int vertCount, int vertPos)
{
	VertDirty = true;
	VertPos = max(VertPos, vertPos + vertCount);
	for (int i = 0; i < vertCount; ++i)
		Vertices[i + vertPos] = vertData[i];
}

void TMesh::SetPositions(const XMFLOAT3* posData, int count)
{
	VertDirty = true;
	VertPos = max(VertPos, count);
	for (int i = 0; i < count; ++i)
		Vertices[i].Position = posData[i];
}

void TMesh::SetColors(const XMFLOAT4* colorData, int count)
{
	VertDirty = true;
	VertPos = max(VertPos, count);
	for (int i = 0; i < count; ++i)
		Vertices[i].Color = colorData[i];
}

void TMesh::SetUVs(const XMFLOAT2* uvData, int count)
{
	VertDirty = true;
	VertPos = max(VertPos, count);
	for (int i = 0; i < count; ++i)
		Vertices[i].UV = uvData[i];
}

void TMesh::SetSubMeshCount(int count)
{
	SubMeshs.resize(count);
}

void TMesh::SetIndices(const UINT* indiceData, int indicePos, int indiceCount, int subMeshIndex)
{
	assert(subMeshIndex < SubMeshs.size());
	IndiceDirty = true;
	for (int i = 0; i < indiceCount; ++i)
		Indices[i + indicePos] = indiceData[i];

	auto& submesh = SubMeshs[subMeshIndex];
	submesh.IndicePos = indicePos;
	submesh.IndiceCount = indiceCount;
}

void TMesh::SetTexture(int slot, ITexturePtr texture, int subMeshIndex)
{
	assert(subMeshIndex < SubMeshs.size());
	auto& submesh = SubMeshs[subMeshIndex];
	submesh.Textures[slot] = texture;
}
