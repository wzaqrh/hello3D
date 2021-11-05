#pragma once
#include "core/rendersys/base_type.h"

namespace mir {

__declspec(align(16)) 
struct TTransform {
	XMMATRIX mMatrix;
	XMFLOAT3 mScale;
	XMFLOAT3 mPosition;
	XMFLOAT3 mEuler;//ZXY
	XMFLOAT3 mFlip;
	bool mDirty = false;
public:
	void* operator new(size_t i) { return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
	TTransform();
public:
	void SetScaleX(float sx);
	void SetScaleY(float sy);
	void SetScaleZ(float sz);
	void SetScale(float s);
	void SetScale(const XMFLOAT3& s);
	XMFLOAT3 GetScale();

	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& position);
	const XMFLOAT3& GetPosition() {
		return mPosition;
	}

	void SetEulerZ(float angle);
	void SetEulerX(float angle);
	void SetEulerY(float angle);
	void SetEuler(const XMFLOAT3& euler);
	const XMFLOAT3& GetEuler() {
		return mEuler;
	}

	void SetFlipY(bool flip);
	bool IsFlipY();

	const XMMATRIX& GetMatrixSRT();
	const XMMATRIX& GetMatrixTSR();
	const XMMATRIX& Matrix();
};
typedef std::shared_ptr<TTransform> TTransformPtr;

struct TMovable : public TTransform {
	float mDefScale;	
public:
	TMovable();
	void SetDefScale(float s);
	const XMMATRIX& GetWorldTransform();
};
typedef std::shared_ptr<TMovable> TMovablePtr;

}