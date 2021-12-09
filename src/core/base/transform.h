#pragma once
#include "core/mir_export.h"
#include "core/base/predeclare.h"
#include "core/base/base_type.h"

#define TRANSFORM_QUATERNION

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
	void SetScale(const Eigen::Vector3f& s);
	void SetPosition(const Eigen::Vector3f& position);
	void SetEuler(const Eigen::Vector3f& euler);
	void SetQuaternion(const Eigen::Quaternionf& quat);
	void SetYFlipped(bool flip);
public:
	const Eigen::Vector3f& GetScale() const { return mScale; }
	const Eigen::Vector3f& GetPosition() const { return mTranslation; }
	RotationOrder GetEulerOrder() const { return kExtrinsicZXY; }
	bool IsYFlipped() const;
	
	bool IsIdentity() const;
	const Eigen::Vector3f& GetForward() const;
	const Eigen::Matrix4f& GetSRT() const;
private:
	void CalculateSRT(Eigen::Matrix4f& matrix) const;
	void CalculateTSR(Eigen::Matrix4f& matrix) const;
	void CalculateForward(Eigen::Vector3f& forward) const;
	void CheckDirtyAndRecalculate() const;
private:
	Eigen::Vector3f mTranslation;
	Eigen::Vector3f mScale;
#if defined TRANSFORM_QUATERNION
	Eigen::Quaternionf mQuat;
#else
	Eigen::Vector3f mEuler;//ZXY
#endif
	Eigen::Vector3f mFlip;

	mutable bool mDirty = false;
	mutable Eigen::Matrix4f mSRT;
	mutable Eigen::Vector3f mForward;
};

}