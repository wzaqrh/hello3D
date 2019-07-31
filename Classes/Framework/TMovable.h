#pragma once
#include "TBaseTypes.h"

struct TMovable {
	float mDefScale;
	XMFLOAT3 mScale;
	XMFLOAT3 mPosition;
	XMFLOAT3 mEuler;//ZXY
public:
	TMovable();
	void SetDefScale(float s);
	void SetScale(float s);
	void SetPosition(float x, float y, float z);
	void SetEulerZ(float angle);
	void SetEulerX(float angle);
	void SetEulerY(float angle);
	XMMATRIX GetWorldTransform();
};
typedef std::shared_ptr<TMovable> TMovablePtr;