#pragma once
#include "core/rendersys/base_type.h"

namespace mir {

#define AS_CONST_REF(TYPE, V) *(const TYPE*)(&V)

struct __declspec(align(16)) Transform {
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
	Transform();
public:
	void SetScaleX(float sx);
	void SetScaleY(float sy);
	void SetScaleZ(float sz);
	void SetScale(float s);
	void SetScale(const XMFLOAT3& s);
	XMFLOAT3 GetScale();

	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& position);
	const XMFLOAT3& GetPosition() { return AS_CONST_REF(XMFLOAT3, mPosition) ; }

	void SetEulerZ(float angle);
	void SetEulerX(float angle);
	void SetEulerY(float angle);
	void SetEuler(const XMFLOAT3& euler);
	const XMFLOAT3& GetEuler() { return AS_CONST_REF(XMFLOAT3, mEuler); }

	void SetFlipY(bool flip);
	bool IsFlipY();

	const XMMATRIX& Matrix();
	const XMMATRIX& SetMatrixSRT();
	const XMMATRIX& SetMatrixTSR();
private:
	Eigen::Matrix4f mMatrix;

	Eigen::Vector3f mScale;
	Eigen::Vector3f mPosition;
	Eigen::Vector3f mEuler;//ZXY
	Eigen::Vector3f mFlip;
	bool mDirty = false;
};
typedef std::shared_ptr<Transform> TransformPtr;

struct Movable : public Transform {
	float mDefScale;	
public:
	Movable();
	void SetDefScale(float s);
	const XMMATRIX& GetWorldTransform();
};
typedef std::shared_ptr<Movable> MovablePtr;

}