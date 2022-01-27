#include "core/base/debug.h"
#include "core/renderable/mesh.h"
#include "core/resource/resource_manager.h"

namespace mir {
namespace rend {

/********** Mesh **********/
cppcoro::shared_task<bool> Mesh::Init(const MaterialLoadParam& loadParam, int vertCount, int indexCount)
{
	COROUTINE_VARIABLES_3(loadParam, vertCount, indexCount);

	if (!co_await Super::Init(loadParam))
		co_return false;

	mIndices.resize(indexCount);
	mIndexBuffer = mResourceMng.CreateIndexBuffer(mLaunchMode, kFormatR32UInt, Data::Make(mIndices));

	mVertices.resize(vertCount);
	mVertexBuffer = mResourceMng.CreateVertexBuffer(mLaunchMode, sizeof(vbSurface), 0, Data::MakeSize(mVertices));
	
	mSubMeshs.resize(1);
	co_return true;
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
		mResourceMng.UpdateBuffer(mVertexBuffer, Data::Make(mVertices));
	}

	if (mIndiceDirty)
	{
		mIndiceDirty = false;
		mResourceMng.UpdateBuffer(mIndexBuffer, Data::Make(mIndices));
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
			op.CameraMask = mCameraMask;
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
}

void Mesh::SetVertexs(const vbSurface* vertData, int vertCount, int vertPos)
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
		mVertices[i].Pos = posData[i];
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