#include <boost/assert.hpp>
#include "core/base/attribute_struct.h"

namespace mir {

/********** SpriteVertexQuad **********/
vbSurfaceQuad::vbSurfaceQuad()
{
	DoSetTexCoords(Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1));
	SetColor(Eigen::Vector4f(1, 1, 1, 1));
}

vbSurfaceQuad::vbSurfaceQuad(const Eigen::Vector2f& origin, const Eigen::Vector2f& size)
{
	SetCornerByRect(origin, size);
	DoSetTexCoords(Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1));
	SetColor(Eigen::Vector4f(1, 1, 1, 1));
}

void vbSurfaceQuad::SetCornerByRect(const Eigen::Vector2f& org, const Eigen::Vector2f& size, float z)
{
	lb().Pos = Eigen::Vector3f(org.x(), org.y(), z);
	lt().Pos = Eigen::Vector3f(org.x(), org.y() + size.y(), z);
	rt().Pos = Eigen::Vector3f(org.x() + size.x(), org.y() + size.y(), z);
	rb().Pos = Eigen::Vector3f(org.x() + size.x(), org.y(), z);
}
void vbSurfaceQuad::SetCornerByLBRT(const Eigen::Vector2f& pLB, const Eigen::Vector2f& pRT, float z)
{
	SetCornerByRect(pLB, pRT - pLB, z);
}

void vbSurfaceQuad::SetCornerByVector(const Eigen::Vector3f& org, const Eigen::Vector3f& right, const Eigen::Vector3f& up)
{
	lb().Pos = org;
	lt().Pos = org + up;
	rt().Pos = org + up + right;
	rb().Pos = org + right;
}

void vbSurfaceQuad::SetColor(const Eigen::Vector4f& color)
{
	unsigned char c[4] = {
		static_cast<unsigned char>(color.x() * 255),
		static_cast<unsigned char>(color.y() * 255),
		static_cast<unsigned char>(color.z() * 255),
		static_cast<unsigned char>(color.w() * 255),
	};
	for (size_t i = 0; i < kCubeConerFrontCount; ++i)
		Coners[i].Color = *((int*)c);
}

void vbSurfaceQuad::SetZ(float z)
{
	for (size_t i = 0; i < kCubeConerFrontCount; ++i)
		Coners[i].Pos.z() = z;
}

void vbSurfaceQuad::FlipY()
{
	std::swap(lb().Tex.y(), rt().Tex.y());
	DoSetTexCoords(lb().Tex, rt().Tex);
}

void vbSurfaceQuad::DoSetTexCoords(Eigen::Vector2f plb, Eigen::Vector2f prt)
{
	lb().Tex = Eigen::Vector2f(plb.x(), plb.y());
	lt().Tex = Eigen::Vector2f(plb.x(), prt.y());
	rt().Tex = Eigen::Vector2f(prt.x(), prt.y());
	rb().Tex = Eigen::Vector2f(prt.x(), plb.y());
}

void vbSurfaceQuad::SetTexCoord(const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1)
{
	DoSetTexCoords(uv0, uv1);
}

constexpr static std::array<uint32_t, 6> vbSQUnitIndices = { 0, 1, 2, 0, 2, 3 };
const std::array<uint32_t, 6>& vbSurfaceQuad::GetIndices() {
	return vbSQUnitIndices;
}

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
void vbSurfaceCube::SetPositionsByCenterHSize(const Eigen::Vector3f& center, const Eigen::Vector3f& size)
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

void vbSurfaceCube::SetColor(const Eigen::Vector4f& color)
{
	for (size_t i = 0; i < kCubeConerCount; ++i) {
		Faces[i].SetColor(color);
	}
}

constexpr static std::array<uint32_t, kCubeConerCount * 6> vbSCUnitIndices = {
#define ILS(I) I*4
#define INDICE_LINE(I) 0+ILS(I), 1+ILS(I), 2+ILS(I), 0+ILS(I), 2+ILS(I), 3+ILS(I)
	INDICE_LINE(0),
	INDICE_LINE(1),
	INDICE_LINE(2),
	INDICE_LINE(3),
	INDICE_LINE(4),
	INDICE_LINE(5)
};
const std::array<uint32_t, kCubeConerCount * 6>& vbSurfaceCube::GetIndices() {
	return vbSCUnitIndices;
}

}