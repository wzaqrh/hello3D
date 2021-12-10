#include "core/base/transform.h"
#include <boost/assert.hpp>

namespace mir {

Transform::Transform()
{
	mScale = math::vec::One();
	mFlip = math::vec::One();
	mTranslation = math::vec::Zero();
#if defined TRANSFORM_QUATERNION
	mQuat = Eigen::Quaternionf::Identity();
#else
	mEuler = math::vec::Zero();
#endif
	mSRT = Eigen::Matrix4f::Identity();
	mForward = math::vec::Forward();
}

void Transform::SetScale(const Eigen::Vector3f& scale)
{
	mScale = scale;
	mDirty = true;
}

void Transform::SetPosition(const Eigen::Vector3f& position)
{
	mTranslation = position;
	mDirty = true;
}

void Transform::SetEuler(const Eigen::Vector3f& euler)
{
#if defined TRANSFORM_QUATERNION
	mQuat = Eigen::AngleAxisf(euler.z(), Eigen::Vector3f::UnitZ())
		* Eigen::AngleAxisf(euler.x(), Eigen::Vector3f::UnitX())
		* Eigen::AngleAxisf(euler.y(), Eigen::Vector3f::UnitY());
#else
	mEuler = euler;
#endif
	mDirty = true;
}

void Transform::SetQuaternion(const Eigen::Quaternionf& quat)
{
	if (quat != Eigen::Quaternionf::Identity()) {
	#if defined TRANSFORM_QUATERNION
		mQuat = quat;
	#endif
		mDirty = true;
	}
}

void Transform::SetYFlipped(bool flip)
{
	mFlip.y() = flip ? -1 : 1;
	mDirty = true;
}

bool Transform::IsYFlipped() const
{
	return mFlip.y() < 0;
}

bool Transform::IsIdentity() const
{
	CheckDirtyAndRecalculate();
	return mSRT.isIdentity();
}

const Eigen::Vector3f& Transform::GetForward() const
{
	CheckDirtyAndRecalculate();
	BOOST_ASSERT(fabs(mForward.norm() - 1.0) < 0.01);
	return mForward;
}

const Eigen::Matrix4f& Transform::GetSRT() const
{
	CheckDirtyAndRecalculate();
	return mSRT;
}

/********** CheckDirtyAndRecalculate **********/
void Transform::CalculateTSR(Eigen::Matrix4f& matrix) const
{
	Transform3fAffine t = Transform3fAffine::Identity();
	t.pretranslate(mTranslation);
	
	t.prescale(mScale);
	t.prescale(mFlip);
	
#if defined TRANSFORM_QUATERNION
	t.prerotate(mQuat);
#else
	if (mEuler.any()) {
		auto euler = Eigen::AngleAxisf(mEuler.z(), Eigen::Vector3f::UnitZ())
			* Eigen::AngleAxisf(mEuler.x(), Eigen::Vector3f::UnitX())
			* Eigen::AngleAxisf(mEuler.y(), Eigen::Vector3f::UnitY());
		t.rotate(euler);
	}
#endif

	matrix = t.matrix();
}

void Transform::CalculateSRT(Eigen::Matrix4f& matrix) const
{
	Transform3fAffine t = Transform3fAffine::Identity();
	t.prescale(mScale);
	t.prescale(mFlip);
	
#if defined TRANSFORM_QUATERNION
	t.prerotate(mQuat);
#else
	if (mEuler.any()) {
		auto euler = Eigen::AngleAxisf(mEuler.z(), Eigen::Vector3f::UnitZ())
			* Eigen::AngleAxisf(mEuler.x(), Eigen::Vector3f::UnitX())
			* Eigen::AngleAxisf(mEuler.y(), Eigen::Vector3f::UnitY());
		t.rotate(euler);
	}
#endif
	
	t.pretranslate(mTranslation);

	matrix = t.matrix();
}

void Transform::CalculateForward(Eigen::Vector3f& forward) const
{
	forward = math::vec::Forward();

#if defined TRANSFORM_QUATERNION
	forward = mQuat * forward;
#else
	if (mEuler.any()) {
		Transform3fAffine t = Transform3fAffine::Identity();

		auto euler = Eigen::AngleAxisf(mEuler.z(), Eigen::Vector3f::UnitZ())
			* Eigen::AngleAxisf(mEuler.x(), Eigen::Vector3f::UnitX())
			* Eigen::AngleAxisf(mEuler.y(), Eigen::Vector3f::UnitY());
		t.rotate(euler);

		forward = t * forward;
	}
#endif
}

void Transform::CheckDirtyAndRecalculate() const
{
	if (mDirty) {
		mDirty = false;
		CalculateSRT(mSRT);
		CalculateForward(mForward);
	}
}
}