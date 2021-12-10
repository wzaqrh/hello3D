#include "core/renderable/cube.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
	
/********** Cube **********/
Cube::Cube(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName)
	: Super(launchMode, resourceMng, matName)
	, mVertexDirty(true)
	, mHalfSize(Eigen::Vector3f::Ones())
	, mPosition(Eigen::Vector3f::Zero())
	, mColor(Eigen::Vector4f::Ones())
{
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

void Cube::GenRenderOperation(RenderOperationQueue& opList)
{
	RenderOperation op = {};
	if (!MakeRenderOperation(op)) return;

	if (mVertexDirty) {
		mVertexDirty = false;
		mVertexData.SetPositionsByCenterHSize(mPosition, mHalfSize);
		mVertexData.SetColor(mColor);
		mResourceMng.UpdateBuffer(mVertexBuffer, Data::Make(mVertexData));
	}

	opList.AddOP(op);
}



}