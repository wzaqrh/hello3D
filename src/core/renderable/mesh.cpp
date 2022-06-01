#include "core/base/debug.h"
#include "core/renderable/mesh.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace rend {

/********** Mesh **********/
Mesh::Mesh(Launch launchMode, ResourceManager& resMng, const res::MaterialInstance& material, int vertCount, int indexCount)
	:Super(launchMode, resMng, material)
{
	mIndices.resize(indexCount);
	mIndexBuffer = mResMng.CreateIndexBuffer(mLaunchMode, kFormatR32UInt, Data::Make(mIndices));

	mVertices.resize(vertCount);
	mVertexBuffer = mResMng.CreateVertexBuffer(mLaunchMode, sizeof(vbSurface), 0, Data::MakeSize(mVertices));
	
	mSubMeshs.resize(1);
}

void Mesh::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded()
		|| !mVertexBuffer->IsLoaded()
		|| !mIndexBuffer->IsLoaded())
		return;

	if (mVertDirty)
	{
		mVertDirty = false;
		mResMng.UpdateBuffer(mVertexBuffer, Data::Make(mVertices));
	}

	if (mIndiceDirty)
	{
		mIndiceDirty = false;
		mResMng.UpdateBuffer(mIndexBuffer, Data::Make(mIndices));
	}

	int opCount = 0;
	for (int i = 0; i < mSubMeshs.size(); ++i) {
		if (mSubMeshs[i].IndiceCount > 0) {
			RenderOperation op = {};
			op.Material = mMaterial;
			op.IndexBuffer = mIndexBuffer;
			op.AddVertexBuffer(mVertexBuffer);
			op.IndexPos = mSubMeshs[i].IndicePos;
			op.IndexCount = mSubMeshs[i].IndiceCount;
			op.IndexBase = mSubMeshs[i].IndiceBase;
			opList.AddOP(op);
			++opCount;
		}
	}
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

void Mesh::SetVertexs(const vbSurface* vertData, int vertCount)
{
	mVertDirty = true;
	mVertPos = vertCount;
	mVertices.assign(vertData, vertData + vertCount);

	mAABB = Eigen::AlignedBox3f();
	for (int i = 0; i < vertCount; ++i)
		mAABB.extend(vertData[i].Pos);
}

void Mesh::SetVertexs(const vbSurface* vertData, int vertCount, int vertPos)
{
	mVertDirty = true;
	mVertPos = std::max(mVertPos, vertPos + vertCount);
	for (int i = 0; i < vertCount; ++i)
		mVertices[i + vertPos] = vertData[i];

	mAABB = Eigen::AlignedBox3f();
	for (int i = 0; i < mVertPos; ++i)
		mAABB.extend(mVertices[i].Pos);
}

void Mesh::SetPositions(const Eigen::Vector3f* posData, int count)
{
	mVertDirty = true;
	mVertPos = std::max(mVertPos, count);
	for (int i = 0; i < mVertPos; ++i)
		mVertices[i].Pos = posData[i];

	mAABB = Eigen::AlignedBox3f();
	for (int i = 0; i < mVertPos; ++i)
		mAABB.extend(mVertices[i].Pos);
}

void Mesh::SetColors(const Eigen::Vector4f* colorData, int count)
{
	mVertDirty = true;
	mVertPos = std::max(mVertPos, count);
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
	mVertPos = std::max(mVertPos, count);
	for (int i = 0; i < mVertPos; ++i)
		mVertices[i].Tex = uvData[i];
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
}