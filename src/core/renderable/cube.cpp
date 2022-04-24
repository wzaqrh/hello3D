#include "core/renderable/cube.h"
#include "core/scene/transform.h"
#include "core/resource/resource_manager.h"
#include "core/base/macros.h"

namespace mir {
namespace rend {

/********** Cube **********/
Cube::Cube(Launch launchMode, ResourceManager& resourceMng, const res::MaterialInstance& material)
	: Super(launchMode, resourceMng, material)
	, mVertexDirty(true)
	, mHalfSize(Eigen::Vector3f::Ones())
	, mPosition(Eigen::Vector3f::Zero())
	, mColor(Eigen::Vector4f::Ones())
{
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(vbSurfaceCube::GetIndices()));
	mVertexBuffer = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(vbSurface), 0, Data::MakeSize(sizeof(mVertexData)));
}

void Cube::SetPosition(const Eigen::Vector3f& center)
{
	Eigen::Vector3f size = IF_AND_OR(! mAABB.isEmpty(), Eigen::Vector3f(mAABB.sizes()), Eigen::Vector3f::Zero());
	mAABB = Eigen::AlignedBox3f();
	mAABB.extend(center - size / 2);
	mAABB.extend(center + size / 2);

	mPosition = center;
	mVertexDirty = true;
}

void Cube::SetHalfSize(const Eigen::Vector3f& hsize)
{
	Eigen::Vector3f center = IF_AND_OR(! mAABB.isEmpty(), Eigen::Vector3f(mAABB.center()), Eigen::Vector3f::Zero());
	mAABB = Eigen::AlignedBox3f();
	mAABB.extend(center - hsize);
	mAABB.extend(center + hsize);

	mHalfSize = hsize;
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
}