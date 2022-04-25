#pragma once
#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <memory>
#include <boost/math/constants/constants.hpp>
#include "core/base/cppcoro.h"

typedef int BOOL;
typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3fAffine;
typedef Eigen::Transform<float, 3, Eigen::Projective> Transform3Projective;

namespace mir {
	
#if !defined EIGEN_DONT_ALIGN_STATICALLY
#define mir_allocator Eigen::aligned_allocator
#define MIR_MAKE_ALIGNED_OPERATOR_NEW EIGEN_MAKE_ALIGNED_OPERATOR_NEW
#else
#define mir_allocator std::allocator
#define MIR_MAKE_ALIGNED_OPERATOR_NEW
#endif

template <class _Instance>
struct CreateInstanceFuncor {
	std::shared_ptr<_Instance> operator()() const {
		return std::allocate_shared<_Instance>(mir_allocator<_Instance>());
	}
	template <typename... T> std::shared_ptr<_Instance> operator()(T &&...args) const {
		return std::allocate_shared<_Instance>(mir_allocator<_Instance>(), std::forward<T>(args)...);
	}
};

template <class _Instance, typename... T> inline std::shared_ptr<_Instance> CreateInstance(T &&...args) {
	return CreateInstanceFuncor<_Instance>()(std::forward<T>(args)...);
}
template <class _Instance, typename... T> inline CoTask<std::shared_ptr<_Instance>> CreateInstanceTask(T &&...args) {
	auto res = CreateInstanceFuncor<_Instance>()(std::forward<T>(args)...);
	if (CoAwait res->Init()) return res;
	else return nullptr;
}


}

namespace mir {
namespace math {
/********** point **********/
namespace point {

inline Eigen::Vector4f One() {
	return Eigen::Vector4f(1, 1, 1, 1);
}
inline Eigen::Vector4f Zero() {
	return Eigen::Vector4f(0, 0, 0, 0);
}
inline Eigen::Vector4f Origin() {
	return Eigen::Vector4f(0, 0, 0, 1);
}
inline Eigen::Vector4f ToLeftHand(const Eigen::Vector4f& p) {
	return Eigen::Vector4f(p.x(), p.y(), -p.z(), p.w());
}
inline Eigen::Vector3f ToLeftHand(const Eigen::Vector3f& p) {
	return Eigen::Vector3f(p.x(), p.y(), -p.z());
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

inline Eigen::Vector4f ToLeftHand(const Eigen::Vector4f& v) {
	return Eigen::Vector4f(v.x(), v.y(), -v.z(), v.w());
}
inline Eigen::Vector3f ToLeftHand(const Eigen::Vector3f& v) {
	return Eigen::Vector3f(v.x(), v.y(), -v.z());
}

namespace anchor {
inline Eigen::Vector3f LeftBottom() {
	return Eigen::Vector3f(0, 0, 0);
}
inline Eigen::Vector3f Left() {
	return Eigen::Vector3f(0, 0.5, 0);
}
inline Eigen::Vector3f LeftTop() {
	return Eigen::Vector3f(0, 1, 0);
}

inline Eigen::Vector3f Bottom() {
	return Eigen::Vector3f(0.5, 0, 0);
}
inline Eigen::Vector3f Center() {
	return Eigen::Vector3f(0.5, 0.5, 0);
}
inline Eigen::Vector3f Top() {
	return Eigen::Vector3f(0.5, 1, 0);
}

inline Eigen::Vector3f RightBottom() {
	return Eigen::Vector3f(1, 0, 0);
}
inline Eigen::Vector3f Right() {
	return Eigen::Vector3f(1, 0.5, 0);
}
inline Eigen::Vector3f RightTop() {
	return Eigen::Vector3f(1, 1, 0);
}
}

}

/********** quat **********/
namespace quat {
inline Eigen::Quaternionf ToLeftHand(const Eigen::Quaternionf& q) {
	float qx = q.x(), qy = q.y(), qz = q.z(), qw = q.w();
	return Eigen::Quaternionf(-qw, qx, qy, -qz);
}
}

/********** camera **********/
namespace cam {
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

inline Eigen::Matrix4f MakePerspectiveLH(float w, float h, float zn, float zf)
{
	const float sh = 2.0f * zn / h;
	const float sw = 2.0f * zn / w;
	const float q = zf / (zf - zn);

	Eigen::Matrix4f projection;
	projection <<
		sw, 0, 0, 0,
		0, sh, 0, 0,
		0,  0, q, -zn * q,
		0,  0, 1, 0;
	return projection;
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
		invWidth + invWidth, 0, 0, -(left + right) * invWidth,
		0, invHeight + invHeight, 0, -(top + bottom) * invHeight,
		0, 0, q, -nearZ * q,
		0, 0, 0, 1;
	return ortho;
}

inline Eigen::Vector3f DefEye() {
	return Eigen::Vector3f(0, 0, -10);
}
inline Eigen::Vector2f DefClippingPlane() {
	return Eigen::Vector2f(0.3, 1000);
}
inline float DefFov() {
	return 60;
}
inline float DefOthoSize() {
	return 5;
}
}

/********** function **********/
inline float ToRadian(float degree) {
	return degree / boost::math::constants::radian<float>();
}
inline float ToDegree(float radian) {
	return radian * boost::math::constants::radian<float>();
}

}
}