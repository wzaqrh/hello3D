#pragma once
#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <Eigen/Geometry>

typedef int BOOL;
typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3fAffine;
typedef Eigen::Transform<float, 3, Eigen::Projective> Transform3Projective;

namespace mir {
namespace math {
	
inline Eigen::Matrix4f MakeLookForwardLH(const Eigen::Vector3f& eye,
	const Eigen::Vector3f& forward,
	const Eigen::Vector3f& up)
{
	Eigen::Vector3f n = /*(at - eye)*/forward.normalized();  //z' = n = normalize(at-eye)
	Eigen::Vector3f u = up.cross(n).normalized(); //x' = u = normalize(up x z');
	Eigen::Vector3f v = n.cross(u);			      //y' = v = z' x x'

	Eigen::Matrix4f view;
	view <<
		u.x(), u.y(), u.z(), -u.dot(eye),
		v.x(), v.y(), v.z(), -v.dot(eye),
		n.x(), n.y(), n.z(), -n.dot(eye),
		0, 0, 0, 1;
	return view;
}

inline Eigen::Matrix4f MakeLookAtLH(const Eigen::Vector3f& eye,
	const Eigen::Vector3f& at,
	const Eigen::Vector3f& up)
{
	return MakeLookForwardLH(eye, at - eye, up);
}

inline Eigen::Matrix4f MakePerspectiveFovLH(float fovY, float aspect, float nearZ, float farZ)
{
	const float h(1.0f / tan(fovY / 2));
	const float w(h / aspect);
	const float q(farZ / (farZ - nearZ));

	Eigen::Matrix4f projection;
	projection << 
	w, 0, 0, 0,
	0, h, 0, 0,
	0, 0, q, -nearZ * q,
	0, 0, 1, 0;
	return projection;
}

inline Eigen::Matrix4f MakeOrthographicOffCenterLH(float left, float right, 
	float bottom, float top, 
	float nearZ, float farZ) {
	float const q(1.0f / (farZ - nearZ));
	float const invWidth(1.0f / (right - left));
	float const invHeight(1.0f / (top - bottom));

	Eigen::Matrix4f ortho;
	ortho <<
	invWidth + invWidth, 0,					    0, -(left + right) * invWidth,
	0,					 invHeight + invHeight, 0, -(top + bottom) * invHeight,
	0,					 0,		                q, -nearZ * q,
	0,					 0,					    0, 1;
	return ortho;
}

/********** point **********/
namespace point {

inline Eigen::Vector4f One() {
	return Eigen::Vector4f(1, 1, 1, 1);
}
inline Eigen::Vector4f Origin() {
	return Eigen::Vector4f(0, 0, 0, 1);
}

}
/********** vec **********/
namespace vec {

inline Eigen::Vector3f One() {
	return Eigen::Vector3f(1, 1, 1);
}
inline Eigen::Vector3f Zero() {
	return Eigen::Vector3f(0, 0, 0);
}

inline Eigen::Vector3f Forward() {
	return Eigen::Vector3f(0, 0, 1);
}
inline Eigen::Vector3f Backward() {
	return Eigen::Vector3f(0, 0, -1);
}
inline Eigen::Vector3f Up() {
	return Eigen::Vector3f(0, 1, 0);
}
inline Eigen::Vector3f Down() {
	return Eigen::Vector3f(0, -1, 0);
}
inline Eigen::Vector3f Right() {
	return Eigen::Vector3f(1, 0, 0);
}
inline Eigen::Vector3f Left() {
	return Eigen::Vector3f(-1, 0, 0);
}

}

/********** camera **********/
namespace cam {

inline Eigen::Vector3f DefEye() {
	return Eigen::Vector3f(0, 0, -10);
}
inline Eigen::Vector3f Zero() {
	return Eigen::Vector3f(0, 0, 0);
}

}
}
}