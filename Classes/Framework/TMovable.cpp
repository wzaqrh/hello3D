#include "TMovable.h"

TMovable::TMovable()
{
	mDefScale = 1;
	mScale = XMFLOAT3(1,1,1);
	mPosition = XMFLOAT3(0, 0, 0);
	mEuler = XMFLOAT3(0, 0, 0);
}

void TMovable::SetDefScale(float s)
{
	mDefScale = s;
	SetScale(s);
}

void TMovable::SetScale(float s)
{
	mScale = XMFLOAT3(s, s, s);
}

void TMovable::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
}

void TMovable::SetEulerZ(float angle)
{
	mEuler.z = angle;
}

void TMovable::SetEulerX(float angle)
{
	mEuler.x = angle;
}

void TMovable::SetEulerY(float angle)
{
	mEuler.y = angle;
}

XMMATRIX TMovable::GetWorldTransform()
{
	XMMATRIX euler = XMMatrixRotationZ(mEuler.z) * XMMatrixRotationX(mEuler.x) * XMMatrixRotationY(mEuler.y);
	return XMMatrixScaling(mScale.x, mScale.y, mScale.z)
		* euler
		* XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
}

