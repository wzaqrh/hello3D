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

enum RelativeSpace {
	kSpaceWorld,
	kSpaceSelf
};

class MIR_CORE_API BaseTransform {
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	BaseTransform();
	
	void SetLocalPosition(const Eigen::Vector3f& position);
	void SetLocalScale(const Eigen::Vector3f& s);

	void SetLocalEulerAngles(const Eigen::Vector3f& euler);
	void SetLocalRotation(const Eigen::Quaternionf& quat);
	void RotateAround(const Eigen::Vector3f& point, const Eigen::Vector3f& axis, float angle);

	void UpdateFrame();
public:
	const Eigen::Vector3f& GetLocalPosition() const { return mPosition; }
	const Eigen::Vector3f& GetLocalScale() const { return mScale; }
	constexpr RotationOrder GetEulerOrder() const { return kExtrinsicZXY; }
	const Eigen::Quaternionf& GetLocalRotation() const { return mQuat; }
	bool HasChanged() const { return mChanged; }
protected:
	void CalculateSRT(Eigen::Matrix4f& matrix) const;
	void CheckDirtyAndRecalculate() const;
	const Eigen::Matrix4f& GetSRT() const;
protected:
	Eigen::Vector3f mPosition;
	Eigen::Vector3f mScale;
	Eigen::Quaternionf mQuat;

	mutable Eigen::Matrix4f mSRT;
	mutable bool mDirty;
	bool mChanged;
};

class MIR_CORE_API Transform : public std::enable_shared_from_this<Transform>, public BaseTransform {
public:
	void SetParent(TransformPtr parent, bool worldPositionStays = true);
	void DetachChildren();

	void SetPosition(const Eigen::Vector3f& position);
	void SetScale(const Eigen::Vector3f& s);
	void SetEulerAngles(const Eigen::Vector3f& euler);
	void SetRotation(const Eigen::Quaternionf& quat);

	void LookAt(const Eigen::Vector3f& at);
	void LookForward(const Eigen::Vector3f& forward);

	void Translate(const Eigen::Vector3f& translation, RelativeSpace relativeTo = kSpaceSelf);
	void Rotate(const Eigen::Vector3f& euler, RelativeSpace relativeTo = kSpaceSelf);
public:
	TransformPtr GetRoot() const;
	TransformPtr GetParent() const { return mParent.lock(); }
	size_t GetChildCount() const { return mChildren.size(); }
	TransformPtr GetChild(size_t index) const;

	Eigen::Vector3f GetPosition() const;
	Eigen::Vector3f GetLossyScale() const;
	Eigen::Quaternionf GetRotation() const;
	Eigen::Matrix4f GetLocalToWorldMatrix() const;
	Eigen::Matrix4f GetWorldToLocalMatrix() const;
	Eigen::Matrix4f GetWorldMatrix() const;

	Eigen::Vector3f GetForward() const;
	Eigen::Vector3f GetRight() const;
	Eigen::Vector3f GetUp() const;
	Eigen::Vector3f GetLookAt() const;
	float GetForwardLength() const { return mForwardLength; }

	Eigen::Vector3f TransformPoint(const Eigen::Vector3f& position) const;
	Eigen::Vector3f TransformVector(const Eigen::Vector3f& vector) const;
	Eigen::Vector3f TransformDirection(const Eigen::Vector3f& direction) const;
	Eigen::Vector3f InverseTransformPoint(const Eigen::Vector3f& position) const;
	Eigen::Vector3f InverseTransformVector(const Eigen::Vector3f& vector) const;
	Eigen::Vector3f InverseTransformDirection(const Eigen::Vector3f& direction) const;
private:
	float mForwardLength = 1.0f;
	TransformWeakPtr mParent;
	std::vector<TransformPtr> mChildren;
};

}