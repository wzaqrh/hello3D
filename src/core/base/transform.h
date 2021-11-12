#pragma once
#include "core/rendersys/base_type.h"
#include "core/base/predeclare.h"

namespace mir {

struct __declspec(align(16)) Transform {
public:
	Transform();
public:
	void SetScale(const Eigen::Vector3f& s);
	const Eigen::Vector3f& GetScale() const { return mScale; }

	void SetPosition(const Eigen::Vector3f& position);
	const Eigen::Vector3f& GetPosition() const { return mPosition; }

	void SetEuler(const Eigen::Vector3f& euler);
	const Eigen::Vector3f& GetEuler() const { return mEuler; }

	void SetFlipY(bool flip);
	bool IsFlipY() const;

	const XMMATRIX& GetMatrix();
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

}