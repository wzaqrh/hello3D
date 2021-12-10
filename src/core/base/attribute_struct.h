#pragma once
#include "core/base/math.h"
#include "core/base/base_type.h"

namespace mir {

struct vbSurface 
{
	vbSurface() :Pos(0, 0, 0), Color(0), Tex(0, 0) {}
public:
	Eigen::Vector3f Pos;
	unsigned int Color;
	Eigen::Vector2f Tex;
};

struct vbSurfaceQuad 
{
	vbSurfaceQuad();
	vbSurfaceQuad(const Eigen::Vector2f& origin, const Eigen::Vector2f& size);
	void SetCornerByRect(const Eigen::Vector2f& origin, const Eigen::Vector2f& size, float z = 0);
	void SetCornerByLBRT(const Eigen::Vector2f& pLB, const Eigen::Vector2f& pRT, float z);
	void SetCornerByVector(const Eigen::Vector3f& pLB, const Eigen::Vector3f& right, const Eigen::Vector3f& up);
	void SetZ(float z);
	void SetColor(const Eigen::Vector4f& color);
	void FlipY();
	void SetTexCoord(const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1);

	vbSurface& lb() { return Coners[kCubeConerFrontLeftBottom]; }
	vbSurface& lt() { return Coners[kCubeConerFrontLeftTop]; }
	vbSurface& rt() { return Coners[kCubeConerFrontRightTop]; }
	vbSurface& rb() { return Coners[kCubeConerFrontRightBottom]; }
	static const std::array<uint32_t, 6>& GetIndices();
private:
	void DoSetTexCoords(Eigen::Vector2f plb, Eigen::Vector2f prt);
private:
	vbSurface Coners[kCubeConerFrontCount];
};

struct vbSurfaceCube
{
	void SetColor(const Eigen::Vector4f& color);
	void SetPositionsByCenterHSize(const Eigen::Vector3f& center, const Eigen::Vector3f& size);
	static const std::array<uint32_t, kCubeConerCount * 6>& GetIndices();
public:
	vbSurfaceQuad Faces[kCubeConerCount];
};

struct vbSkeleton
{
	vbSkeleton() 
	: Normal(0, 0, 0), Tangent(0, 0, 0), BiTangent(0, 0, 0)
	, BlendWeights(0, 0, 0, 0), BlendIndices(0, 0, 0, 0) 
	{}
public:
	Eigen::Vector3f Normal;
	Eigen::Vector3f Tangent;
	Eigen::Vector3f BiTangent;

	Eigen::Vector4f BlendWeights;
	Eigen::Vector4i BlendIndices;
};

}