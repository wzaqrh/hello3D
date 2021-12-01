#include "core/renderable/cube.h"
#include "core/base/transform.h"
#include "core/resource/resource_manager.h"

namespace mir {
	

/********** CubeVertexSixFace **********/
static void GenCubeCorners(const Eigen::Vector3f& pos, const Eigen::Vector3f& size, Eigen::Vector3f corners[kCubeConerCount])
{
	for (size_t i = 0; i < kCubeConerCount; ++i) {
		bool isRight = IS_CORNER_RIGHT(i);
		bool isTop = IS_CORNER_TOP(i);
		bool isBack = IS_CORNER_BACK(i);
		corners[i].x() = isRight ? pos.x() + size.x() : pos.x() - size.x();
		corners[i].y() = isTop ? pos.y() + size.y() : pos.y() - size.y();
		corners[i].z() = isBack ? pos.z() + size.z() : pos.z() - size.z();
	}
}
void CubeVertexSixFace::SetPositionsByCenterHSize(const Eigen::Vector3f& center, const Eigen::Vector3f& size)
{
	BOOST_ASSERT(size.x() > 0 && size.y() > 0 && size.z() > 0);

	Eigen::Vector3f corners[kCubeConerCount];
	GenCubeCorners(center, size, corners);

	const Eigen::Vector3f right(size.x() * 2, 0, 0);
	const Eigen::Vector3f up(0, size.y() * 2, 0);
	const Eigen::Vector3f forward(0, 0, size.z() * 2);

	Faces[kCubeFaceNegX].SetCornerByVector(corners[kCubeConerBackLeftBottom], -forward, up);
	Faces[kCubeFacePosX].SetCornerByVector(corners[kCubeConerFrontRightBottom], forward, up);

	Faces[kCubeFaceNegY].SetCornerByVector(corners[kCubeConerFrontRightBottom], -right, forward);
	Faces[kCubeFacePosY].SetCornerByVector(corners[kCubeConerFrontLeftTop], right, forward);

	Faces[kCubeFacePosZ].SetCornerByVector(corners[kCubeConerBackRightBottom], -right, up);
	Faces[kCubeFaceNegZ].SetCornerByVector(corners[kCubeConerFrontLeftBottom], right, up);
}

void CubeVertexSixFace::SetColor(const Eigen::Vector4f& color)
{
	for (size_t i = 0; i < kCubeConerCount; ++i) {
		Faces[i].SetColor(color);
	}
}

constexpr uint32_t CCubeIndices[] = {
#define ILS(I) I*4
#define INDICE_LINE(I) 0+ILS(I), 1+ILS(I), 2+ILS(I), 0+ILS(I), 2+ILS(I), 3+ILS(I)
	INDICE_LINE(0),
	INDICE_LINE(1),
	INDICE_LINE(2),
	INDICE_LINE(3),
	INDICE_LINE(4),
	INDICE_LINE(5)
};

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
	mIndexBuffer = resourceMng.CreateIndexBuffer(__launchMode__, kFormatR32UInt, Data::Make(CCubeIndices));
	mVertexBuffer = resourceMng.CreateVertexBuffer(__launchMode__, sizeof(SpriteVertex), 0, Data::MakeSize(sizeof(mVertexData)));
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
	op.VertexBuffer = mVertexBuffer;
	op.WorldTransform = mTransform->GetMatrix();
	op.CameraMask = mCameraMask;
	opList.AddOP(op);
	return 1;
}



}