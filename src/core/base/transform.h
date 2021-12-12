#pragma once
#include "core/mir_export.h"
#include "core/base/predeclare.h"
#include "core/base/base_type.h"

namespace mir {

enum RotationOrder {
	kIntrinsicXYZ,
	kIntrinsicYZX,
	kIntrinsicZXY,
	kIntrinsicXZY,
	kIntrinsicZYX,
	kIntrinsicYXZ,

	kExtrinsicZYX,
	kExtrinsicXZY,
	kExtrinsicYXZ,
	kExtrinsicYZX,
	kExtrinsicXYZ,
	kExtrinsicZXY,

	kExtrinsicFirst = kExtrinsicZYX
};

struct MIR_CORE_API Transform {
public:
	Transform();
	
	void SetPosition(const Eigen::Vector3f& position);
	void Translate(const Eigen::Vector3f& translation);

	void SetScale(const Eigen::Vector3f& s);

	void SetEuler(const Eigen::Vector3f& euler);
	void SetRotation(const Eigen::Quaternionf& quat);
	void Rotate(const Eigen::Vector3f& euler);
	void RotateAround(const Eigen::Vector3f& point, const Eigen::Vector3f& axis, float angle);
public:
	const Eigen::Vector3f& GetPosition() const { return mPosition; }
	const Eigen::Vector3f& GetScale() const { return mScale; }
	RotationOrder GetEulerOrder() const { return kExtrinsicZXY; }
	const Eigen::Quaternionf& GetRotation() const { return mQuat; }
	
	bool IsIdentity() const;
	const Eigen::Matrix4f& GetSRT() const;
	Eigen::Vector3f GetForward() const;
	Eigen::Vector3f GetRight() const;
	Eigen::Vector3f GetUp() const;
private:
	void CalculateSRT(Eigen::Matrix4f& matrix) const;
	void CalculateTSR(Eigen::Matrix4f& matrix) const;
	void CheckDirtyAndRecalculate() const;
private:
	Eigen::Vector3f mPosition;
	Eigen::Vector3f mScale;
	Eigen::Quaternionf mQuat;

	mutable Eigen::Matrix4f mSRT;
	mutable bool mDirty = false;
};

}