#include "core/base/transform.h"
#include <boost/assert.hpp>

namespace mir {

//TTransform
Transform::Transform()
{
	mScale = Eigen::Vector3f(1, 1, 1);
	mFlip = Eigen::Vector3f(1, 1, 1);
	mPosition = Eigen::Vector3f(0, 0, 0);
	mEuler = Eigen::Vector3f(0, 0, 0);
	mMatrix = Eigen::Matrix4f::Identity();
}

void Transform::SetScaleX(float sx)
{
	mScale.x() = sx;
	mDirty = true;
}

void Transform::SetScaleY(float sy)
{
	mScale.y() = sy;
	mDirty = true;
}

void Transform::SetScaleZ(float sz)
{
	mScale.z() = sz;
	mDirty = true;
}

void Transform::SetScale(float s)
{
	mScale = Eigen::Vector3f(s, s, s);
	mDirty = true;
}

void Transform::SetScale(const XMFLOAT3& s)
{
	mScale = AS_CONST_REF(Eigen::Vector3f, s);
	mDirty = true;
}

XMFLOAT3 Transform::GetScale()
{
	Eigen::Vector3f scale = mScale.cwiseProduct(mFlip);
	return AS_CONST_REF(XMFLOAT3, scale);
}

void Transform::SetPosition(float x, float y, float z)
{
	mPosition = Eigen::Vector3f(x, y, z);
	mDirty = true;
}

void Transform::SetPosition(const XMFLOAT3& position)
{
	mPosition = AS_CONST_REF(Eigen::Vector3f, position);
	mDirty = true;
}

void Transform::SetEulerZ(float angle)
{
	mEuler.z() = angle;
	mDirty = true;
}

void Transform::SetEulerX(float angle)
{
	mEuler.x() = angle;
	mDirty = true;
}

void Transform::SetEulerY(float angle)
{
	mEuler.y() = angle;
	mDirty = true;
}

void Transform::SetEuler(const XMFLOAT3& euler)
{
	mEuler = AS_CONST_REF(Eigen::Vector3f, euler);
	mDirty = true;
}

void Transform::SetFlipY(bool flip)
{
	mFlip.y() = flip ? -1 : 1;
	mDirty = true;
}

bool Transform::IsFlipY()
{
	return mFlip.y() < 0;
}

const XMMATRIX& Transform::SetMatrixSRT()
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
	return AS_CONST_REF(XMMATRIX, mMatrix);
}

const XMMATRIX& Transform::Matrix()
{
	return AS_CONST_REF(XMMATRIX, SetMatrixSRT());
}

const XMMATRIX& Transform::SetMatrixTSR()
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
	return AS_CONST_REF(XMMATRIX, mMatrix);
}

//TMovable
Movable::Movable()
{
	mDefScale = 1;
}

void Movable::SetDefScale(float s)
{
	mDefScale = s;
	SetScale(s);
}

const XMMATRIX& Movable::GetWorldTransform()
{
	return AS_CONST_REF(XMMATRIX, SetMatrixSRT());
}

}