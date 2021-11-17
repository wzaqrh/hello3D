#include "core/renderable/mesh.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** Mesh **********/
Mesh::Mesh(ResourceManager& resourceMng, MaterialFactory& matFac, const std::string& matName, int vertCount, int indexCount)
	:mResourceMng(resourceMng)
{
	mMaterial = matFac.GetMaterial(matName != "" ? matName : E_MAT_SPRITE);

	mVertices.resize(vertCount);
	mVertexBuffer = mResourceMng.CreateVertexBuffer(sizeof(MeshVertex) * vertCount, sizeof(MeshVertex), 0, nullptr);
	
	mIndices.resize(indexCount);
	mIndexBuffer = mResourceMng.CreateIndexBuffer(sizeof(UINT) * indexCount, kFormatR32UInt, &mIndices[0]);
	
	mSubMeshs.resize(1);
}

int Mesh::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded()
		|| !mVertexBuffer->IsLoaded()
		|| !mIndexBuffer->IsLoaded())
		return 0;

	if (mVertDirty)
	{
		mVertDirty = false;
		mResourceMng.UpdateBuffer(mVertexBuffer, &mVertices[0], mVertices.size() * sizeof(MeshVertex));
	}

	if (mIndiceDirty)
	{
		mIndiceDirty = false;
		mResourceMng.UpdateBuffer(mIndexBuffer, &mIndices[0], mIndices.size() * sizeof(UINT));
	}

	int opCount = 0;
	for (int i = 0; i < mSubMeshs.size(); ++i)
	if (mSubMeshs[i].IndiceCount > 0)
	{
		RenderOperation op = {};
		op.mMaterial = mMaterial;
		op.mIndexBuffer = mIndexBuffer;
		op.mVertexBuffer = mVertexBuffer;
		op.mTextures = mSubMeshs[i].Textures;
		op.mIndexPos = mSubMeshs[i].IndicePos;
		op.mIndexCount = mSubMeshs[i].IndiceCount;
		op.mIndexBase = mSubMeshs[i].IndiceBase;
		opList.AddOP(op);
		++opCount;
	}
	return opCount;
}

void Mesh::Clear()
{
	mVertDirty = true;
	mVertPos = 0;

	mIndiceDirty = true;
	mSubMeshs.resize(1);
	mSubMeshs[0].IndiceCount = 0;
	mSubMeshs[0].IndicePos = 0;
}

void Mesh::SetVertexs(const MeshVertex* vertData, int vertCount)
{
	mVertDirty = true;
	mVertPos = vertCount;
	mVertices.assign(vertData, vertData + vertCount);
}

void Mesh::SetVertexs(const MeshVertex* vertData, int vertCount, int vertPos)
{
	mVertDirty = true;
	mVertPos = max(mVertPos, vertPos + vertCount);
	for (int i = 0; i < vertCount; ++i)
		mVertices[i + vertPos] = vertData[i];
}

void Mesh::SetPositions(const Eigen::Vector3f* posData, int count)
{
	mVertDirty = true;
	mVertPos = max(mVertPos, count);
	for (int i = 0; i < mVertPos; ++i)
		mVertices[i].Position = posData[i];
}

void Mesh::SetColors(const Eigen::Vector4f* colorData, int count)
{
	mVertDirty = true;
	mVertPos = max(mVertPos, count);
	for (int i = 0; i < mVertPos; ++i) {
		unsigned char c[4] = {
			static_cast<unsigned char>(colorData[i].x() * 255),
			static_cast<unsigned char>(colorData[i].y() * 255),
			static_cast<unsigned char>(colorData[i].z() * 255),
			static_cast<unsigned char>(colorData[i].w() * 255),
		};
		mVertices[i].Color = *((int*)c);
	}
}

void Mesh::SetUVs(const Eigen::Vector2f* uvData, int count)
{
	mVertDirty = true;
	mVertPos = max(mVertPos, count);
	for (int i = 0; i < mVertPos; ++i)
		mVertices[i].UV = uvData[i];
}

void Mesh::SetSubMeshCount(int count)
{
	mSubMeshs.resize(count);
}

void Mesh::SetIndices(const unsigned int* indiceData, int indicePos, int indiceCount, int indiceBase, int subMeshIndex)
{
	assert(subMeshIndex < mSubMeshs.size());
	mIndiceDirty = true;
	for (int i = 0; i < indiceCount; ++i)
		mIndices[i + indicePos] = indiceData[i];

	auto& submesh = mSubMeshs[subMeshIndex];
	submesh.IndicePos = indicePos;
	submesh.IndiceCount = indiceCount;
	submesh.IndiceBase = indiceBase;
}

void Mesh::SetTexture(int slot, ITexturePtr texture, int subMeshIndex)
{
	assert(subMeshIndex < mSubMeshs.size());
	auto& submesh = mSubMeshs[subMeshIndex];
	submesh.Textures[slot] = texture;
}

}