#include "core/base/transform.h"

namespace mir {

//TTransform
Transform::Transform()
{
	mScale = XMFLOAT3(1, 1, 1);
	mFlip = XMFLOAT3(1, 1, 1);
	mPosition = XMFLOAT3(0, 0, 0);
	mEuler = XMFLOAT3(0, 0, 0);
	mMatrix = XMMatrixIdentity();
}

void Transform::SetScaleX(float sx)
{
	mScale.x = sx;
	mDirty = true;
}

void Transform::SetScaleY(float sy)
{
	mScale.y = sy;
	mDirty = true;
}

void Transform::SetScaleZ(float sz)
{
	mScale.z = sz;
	mDirty = true;
}

void Transform::SetScale(float s)
{
	mScale = XMFLOAT3(s, s, s);
	mDirty = true;
}

void Transform::SetScale(const XMFLOAT3& s)
{
	mScale = s;
	mDirty = true;
}

XMFLOAT3 Transform::GetScale()
{
	return XMFLOAT3(mScale.x * mFlip.x, mScale.y * mFlip.y, mScale.z * mFlip.z);
}

void Transform::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
	mDirty = true;
}

void Transform::SetPosition(const XMFLOAT3& position)
{
	mPosition = position;
	mDirty = true;
}

void Transform::SetEulerZ(float angle)
{
	mEuler.z = angle;
	mDirty = true;
}

void Transform::SetEulerX(float angle)
{
	mEuler.x = angle;
	mDirty = true;
}

void Transform::SetEulerY(float angle)
{
	mEuler.y = angle;
	mDirty = true;
}

void Transform::SetEuler(const XMFLOAT3& euler)
{
	mEuler = euler;
	mDirty = true;
}

void Transform::SetFlipY(bool flip)
{
	mFlip.y = flip ? -1 : 1;
	mDirty = true;
}

bool Transform::IsFlipY()
{
	return mFlip.y < 0;
}

const XMMATRIX& Transform::GetMatrixSRT()
{
	if (mDirty)
	{
		mDirty = false;
		mMatrix = XMMatrixScaling(mScale.x, mScale.y, mScale.z);
		if (mFlip.x != 1 || mFlip.y != 1 || mFlip.z != 1) {
			mMatrix *= XMMatrixScaling(mFlip.x, mFlip.y, mFlip.z);
		}
		if (mEuler.x != 0 || mEuler.x != 0 || mEuler.z != 0) {
			XMMATRIX euler = XMMatrixRotationZ(mEuler.z) * XMMatrixRotationX(mEuler.x) * XMMatrixRotationY(mEuler.y);
			mMatrix *= euler;
		}
		if (mPosition.x != 0 || mPosition.y != 0 || mPosition.z != 0) {
			mMatrix *= XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		}
	}
	return mMatrix;
}

const XMMATRIX& Transform::Matrix()
{
	return GetMatrixSRT();
}

const XMMATRIX& Transform::GetMatrixTSR()
{
	if (mDirty)
	{
		mDirty = false;
		mMatrix = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
		if (mScale.x != 1 || mScale.y != 1 || mScale.z != 1) {
			mMatrix *= XMMatrixScaling(mScale.x, mScale.y, mScale.z);
		}
		if (mFlip.x != 1 || mFlip.y != 1 || mFlip.z != 1) {
			mMatrix *= XMMatrixScaling(mFlip.x, mFlip.y, mFlip.z);
		}
		if (mEuler.x != 0 || mEuler.x != 0 || mEuler.z != 0) {
			XMMATRIX euler = XMMatrixRotationZ(mEuler.z) * XMMatrixRotationX(mEuler.x) * XMMatrixRotationY(mEuler.y);
			mMatrix *= euler;
		}
	}
	return mMatrix;
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
	return GetMatrixSRT();
}

}