#include <boost/assert.hpp>
#include <boost/math/constants/constants.hpp>
#include "core/base/transform.h"

namespace mir {

Transform::Transform()
{
	mScale = math::vec::One();
	mPosition = math::vec::Zero();
	mQuat = Eigen::Quaternionf::Identity();
	mSRT = Eigen::Matrix4f::Identity();
}

void Transform::SetPosition(const Eigen::Vector3f& position)
{
	mPosition = position;
	mDirty = true;
}
void Transform::Translate(const Eigen::Vector3f& translation)
{
	mPosition += translation;
	mDirty = true;
}

void Transform::SetScale(const Eigen::Vector3f& scale)
{
	mScale = scale;
	mDirty = true;
}

void Transform::SetRotation(const Eigen::Quaternionf& quat)
{
	mQuat = quat;
	mDirty = true;
}
void Transform::SetEuler(const Eigen::Vector3f& euler)
{
	mQuat = Eigen::AngleAxisf(euler.z(), Eigen::Vector3f::UnitZ())
		* Eigen::AngleAxisf(euler.x(), Eigen::Vector3f::UnitX())
		* Eigen::AngleAxisf(euler.y(), Eigen::Vector3f::UnitY());
	mDirty = true;
}
void Transform::Rotate(const Eigen::Vector3f& euler)
{
	auto quat = Eigen::AngleAxisf(euler.z(), Eigen::Vector3f::UnitZ())
		* Eigen::AngleAxisf(euler.x(), Eigen::Vector3f::UnitX())
		* Eigen::AngleAxisf(euler.y(), Eigen::Vector3f::UnitY());
	//mPosition = quat * mPosition;
	mQuat = quat;
	mDirty = true;
}
void Transform::RotateAround(const Eigen::Vector3f& point, const Eigen::Vector3f& axis, float angle)
{
	auto quat = Eigen::AngleAxisf(angle / boost::math::constants::radian<float>(), axis);
	mPosition = point + quat * (mPosition - point);
}

bool Transform::IsIdentity() const
{
	return GetSRT().isIdentity();
}
const Eigen::Matrix4f& Transform::GetSRT() const
{
	CheckDirtyAndRecalculate();
	return mSRT;
}

Eigen::Vector3f Transform::GetForward() const 
{
	auto forward = mQuat * math::vec::Forward();
	BOOST_ASSERT(fabs(forward.norm() - 1.0) < 0.01);
	return forward;
}
Eigen::Vector3f Transform::GetRight() const 
{
	auto right = mQuat * math::vec::Right();
	BOOST_ASSERT(fabs(right.norm() - 1.0) < 0.01);
	return right;
}
Eigen::Vector3f Transform::GetUp() const 
{
	auto up = mQuat * math::vec::Up();
	BOOST_ASSERT(fabs(up.norm() - 1.0) < 0.01);
	return up;
}

/********** CheckDirtyAndRecalculate **********/
void Transform::CalculateTSR(Eigen::Matrix4f& matrix) const
{
	Transform3fAffine t = Transform3fAffine::Identity();
	t.pretranslate(mPosition);
	
	t.prescale(mScale);
	
	t.prerotate(mQuat);
	matrix = t.matrix();
}

void Transform::CalculateSRT(Eigen::Matrix4f& matrix) const
{
	Transform3fAffine t = Transform3fAffine::Identity();
	t.prescale(mScale);
	
	t.prerotate(mQuat);
	
	t.pretranslate(mPosition);
	matrix = t.matrix();
}

void Transform::CheckDirtyAndRecalculate() const
{
	if (mDirty) {
		mDirty = false;
		CalculateSRT(mSRT);
	}
}
}