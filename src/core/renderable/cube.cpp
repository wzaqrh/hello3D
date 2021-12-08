#include "core/renderable/cube.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
	
/********** Cube **********/
Cube::Cube(Launch launchMode, ResourceManager& resourceMng, const std::string& matName)
	: mResourceMng(resourceMng)
	, mVertexDirty(true)
	, mHalfSize(1, 1, 1)
	, mPosition(0, 0, 0)
	, mColor(1, 1, 1, 1)
{
	mTransform = std::make_shared<Transform>();
	mMaterial = resourceMng.CreateMaterial(__launchMode__, matName != "" ? matName : E_MAT_SPRITE);
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(vbSurfaceCube::GetIndices()));
	mVertexBuffer = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::MakeSize(sizeof(mVertexData)));
}

void Cube::SetPosition(const Eigen::Vector3f& pos)
{
	mPosition = pos;
	mVertexDirty = true;
}

void Cube::SetHalfSize(const Eigen::Vector3f& size)
{
	mHalfSize = size;
	mVertexDirty = true;
}

void Cube::SetColor(const Eigen::Vector4f& color)
{
	mColor = color;
	mVertexDirty = true;
}

void Cube::SetColor(unsigned bgra)
{
	unsigned char* cc = (unsigned char*)&bgra;
	mColor = Eigen::Vector4f(cc[2], cc[1], cc[0], cc[3])  / 255.0f;
	mVertexDirty = true;
}

int Cube::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded()
		|| !mVertexBuffer->IsLoaded()
		|| !mIndexBuffer->IsLoaded())
		return 0;

	if (mVertexDirty) {
		mVertexDirty = false;
		mVertexData.SetPositionsByCenterHSize(mPosition, mHalfSize);
		mVertexData.SetColor(mColor);
		mResourceMng.UpdateBuffer(mVertexBuffer, Data::Make(mVertexData));
	}

	RenderOperation op = {};
	op.Material = mMaterial;
	op.IndexBuffer = mIndexBuffer;
	op.AddVertexBuffer(mVertexBuffer);
	op.WorldTransform = mTransform->GetSRT();
	op.CameraMask = mCameraMask;
	opList.AddOP(op);
	return 1;
}



}