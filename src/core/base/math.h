#pragma once
#include "std.h"

namespace mir {
namespace math {
	
inline Eigen::Matrix4f MakeLookAtLH(const Eigen::Vector3f& eye,
	const Eigen::Vector3f& at,
	const Eigen::Vector3f& up)
{
	Eigen::Matrix4f view;

	Eigen::Vector3f n = (at - eye).normalized(); //z' = n = normalize(at-eye)
	Eigen::Vector3f u = up.cross(n).normalized();//x' = u = normalize(up x z');
	Eigen::Vector3f v = n.cross(u);				 //y' = v = z' x x'
	view.block<3, 1>(0, 0) = u;
	view.block<3, 1>(0, 1) = v;
	view.block<3, 1>(0, 2) = n;

	view(0, 3) = -u.dot(eye);
	view(1, 3) = -v.dot(eye);
	view(2, 3) = -n.dot(eye);

	view(3, 0) = 0;
	view(3, 1) = 0;
	view(3, 2) = 0;
	view(3, 3) = 1;
	return view;
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
	invWidth + invWidth, 0,   0, -(left + right) * invWidth,
	0, invHeight + invHeight, 0, -(top + bottom) * invHeight,
	0, 0,		              q, -nearZ * q,
	0, 0,					  0, 1;
	return ortho;
}

}
}