#include "core/base/transform.h"
#include <boost/assert.hpp>

namespace mir {

Transform::Transform()
{
	mScale = Eigen::Vector3f(1, 1, 1);
	mFlip = Eigen::Vector3f(1, 1, 1);
	mPosition = Eigen::Vector3f(0, 0, 0);
	mEuler = Eigen::Vector3f(0, 0, 0);
	mMatrix = Eigen::Matrix4f::Identity();
}

void Transform::SetScale(const Eigen::Vector3f& s)
{
	mScale = s;
	mDirty = true;
}

void Transform::SetPosition(const Eigen::Vector3f& position)
{
	mPosition = position;
	mDirty = true;
}

void Transform::SetEuler(const Eigen::Vector3f& euler)
{
	mEuler = euler;
	mDirty = true;
}

void Transform::SetFlipY(bool flip)
{
	mFlip.y() = flip ? -1 : 1;
	mDirty = true;
}

bool Transform::IsFlipY() const
{
	return mFlip.y() < 0;
}

const Eigen::Matrix4f& Transform::SetMatrixSRT()
{
	if (mDirty)
	{
		mDirty = false;

		Transform3fAffine srt = Transform3fAffine::Identity();
		srt.scale(mScale);
		if (mFlip.x() != 1 || mFlip.y() != 1 || mFlip.z() != 1)
			srt.scale(mFlip);
		if (mEuler.x() != 0 || mEuler.x() != 0 || mEuler.z() != 0) {
			auto euler = Eigen::AngleAxisf(mEuler.z(), Eigen::Vector3f::UnitZ())
				* Eigen::AngleAxisf(mEuler.x(), Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(mEuler.y(), Eigen::Vector3f::UnitY());
			srt.rotate(euler);
		}
		if (mPosition.x() != 0 || mPosition.y() != 0 || mPosition.z() != 0)
			srt.pretranslate(mPosition);

		mMatrix = srt.matrix();
	}
	return mMatrix;
}

const Eigen::Matrix4f& Transform::SetMatrixTSR()
{
	if (mDirty)
	{
		mDirty = false;

		Transform3fAffine tsr = Transform3fAffine::Identity();
		tsr.translate(mPosition);
		if (mScale.x() != 1 || mScale.y() != 1 || mScale.z() != 1)
			tsr.scale(mScale);
		if (mFlip.x() != 1 || mFlip.y() != 1 || mFlip.z() != 1)
			tsr.scale(mFlip);
		if (mEuler.x() != 0 || mEuler.x() != 0 || mEuler.z() != 0) {
			auto euler = Eigen::AngleAxisf(mEuler.z(), Eigen::Vector3f::UnitZ())
				* Eigen::AngleAxisf(mEuler.x(), Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(mEuler.y(), Eigen::Vector3f::UnitY());
			tsr.rotate(euler);
		}

		mMatrix = tsr.matrix();
	}
	return mMatrix;
}

const Eigen::Matrix4f& Transform::GetMatrix()
{
	return SetMatrixSRT();
}

}