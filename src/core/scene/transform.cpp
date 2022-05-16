#include <boost/assert.hpp>
#include <boost/math/constants/constants.hpp>
#include "core/scene/transform.h"

namespace mir {

BaseTransform::BaseTransform()
{
	mScale = math::vec::One();
	mPosition = math::vec::Zero();
	mQuat = Eigen::Quaternionf::Identity();
	mSRT = Eigen::Matrix4f::Identity();
}

void BaseTransform::SetLocalPosition(const Eigen::Vector3f& position)
{
	mPosition = position;
	mSignal();
}

void BaseTransform::SetLocalScale(const Eigen::Vector3f& scale)
{
	mScale = scale;
	mSignal();
}

void BaseTransform::SetLocalRotation(const Eigen::Quaternionf& quat)
{
	mQuat = quat;
	mSignal();
}
void BaseTransform::SetLocalEulerAngles(const Eigen::Vector3f& euler)
{
	this->SetLocalRotation(Eigen::AngleAxisf(euler.z(), Eigen::Vector3f::UnitZ())
		* Eigen::AngleAxisf(euler.x(), Eigen::Vector3f::UnitX())
		* Eigen::AngleAxisf(euler.y(), Eigen::Vector3f::UnitY()));
}
void BaseTransform::RotateAround(const Eigen::Vector3f& point, const Eigen::Vector3f& axis, float angle)
{
	auto quat = Eigen::AngleAxisf(angle / boost::math::constants::radian<float>(), axis);
	mPosition = point + quat * (mPosition - point);
	mSignal();
}

CoTask<void> BaseTransform::UpdateFrame()
{
	CoReturn;
}

#if 0
void BaseTransform::CalculateTSR(Eigen::Matrix4f& matrix) const
{
	Transform3fAffine t = Transform3fAffine::Identity();
	t.pretranslate(mPosition);
	
	t.prescale(mScale);
	
	t.prerotate(mQuat);
	matrix = t.matrix();
}
#endif

void BaseTransform::CalculateSRT(Eigen::Matrix4f& matrix) const
{
	Transform3fAffine t = Transform3fAffine::Identity();
	t.prescale(mScale);
	
	t.prerotate(mQuat);
	
	t.pretranslate(mPosition);
	matrix = t.matrix();
}
const Eigen::Matrix4f& BaseTransform::GetSRT() const
{
	if (mSignal.Slot.AcquireSignal()) 
		CalculateSRT(mSRT);
	return mSRT;
}

/********** Transform **********/
void Transform::DetachChildren()
{
	for (auto& child : mChildren)
		child->mParent.reset();
	mChildren.clear();
}
void Transform::SetParent(TransformPtr parent, bool worldPositionStays)
{
	TransformPtr prevParent = mParent.lock();
	if (parent.get() == prevParent.get()) return;

	Eigen::Matrix4f wm = GetWorldMatrix();
	if (prevParent) {
		prevParent->mChildren.erase(std::remove_if(prevParent->mChildren.begin(), prevParent->mChildren.end(), [this](const TransformPtr& child) {
			return child.get() == this;
		}), prevParent->mChildren.end());
	}

	mParent = parent;

	if (parent) {
		parent->mChildren.push_back(shared_from_this());
	}

	if (worldPositionStays) {
		Transform3fAffine t(wm);
		t *= GetWorldToLocalMatrix();

		SetLocalPosition(t.translation());
		SetLocalRotation(Eigen::Quaternionf(t.rotation()));
		SetLocalScale(t.affine().diagonal().head<3>());
	}

	mSignal();
}

void Transform::SetPosition(const Eigen::Vector3f& position)
{
	if (auto parent = mParent.lock()) {
		mPosition = position - parent->GetPosition();
	}
	else {
		mPosition = position;
	}
	mSignal();
}
void Transform::SetScale(const Eigen::Vector3f& s)
{
	if (auto parent = mParent.lock()) {
		Eigen::Vector3f ls = parent->GetLossyScale();
		mScale = Eigen::Vector3f(s.x() / ls.x(), s.y() / ls.y(), s.z() / ls.z());
	}
	else {
		mScale = s;
	}
	mSignal();
}
void Transform::SetRotation(const Eigen::Quaternionf& quat)
{
	if (auto parent = mParent.lock()) {
		Transform3fAffine t(quat);
		t.rotate(parent->GetRotation().inverse());
		mQuat = t.rotation();
	}
	else {
		mQuat = quat;
	}
	mSignal();
}
void Transform::SetEulerAngles(const Eigen::Vector3f& euler)
{
	this->SetLocalRotation(Eigen::AngleAxisf(euler.z(), Eigen::Vector3f::UnitZ())
		* Eigen::AngleAxisf(euler.x(), Eigen::Vector3f::UnitX())
		* Eigen::AngleAxisf(euler.y(), Eigen::Vector3f::UnitY()));
}

void Transform::LookAt(const Eigen::Vector3f& at)
{
	auto forward = at - GetPosition();
	this->SetRotation(Eigen::Quaternionf::FromTwoVectors(math::vec::Forward(), forward));
	mForwardLength = forward.norm();
	mSignal();
}
void Transform::LookForward(const Eigen::Vector3f& forward)
{
	this->SetRotation(Eigen::Quaternionf::FromTwoVectors(math::vec::Forward(), forward));
	mForwardLength = 1;
	mSignal();
}

void Transform::Translate(const Eigen::Vector3f& translation, RelativeSpace relativeTo)
{
	if (relativeTo == kSpaceSelf) {
		mPosition += translation;
	}
	else {
		Transform3fAffine t(GetSRT());
		Eigen::Matrix4f l2w = GetLocalToWorldMatrix(); t *= l2w;
		t.translate(translation);
		Eigen::Matrix4f w2l = GetWorldToLocalMatrix(); t *= w2l;

		mPosition = t.translation();
	}
	mSignal();
}
void Transform::Rotate(const Eigen::Vector3f& euler, RelativeSpace relativeTo)
{
	auto quat = Eigen::AngleAxisf(euler.z(), Eigen::Vector3f::UnitZ())
		* Eigen::AngleAxisf(euler.x(), Eigen::Vector3f::UnitX())
		* Eigen::AngleAxisf(euler.y(), Eigen::Vector3f::UnitY());

	if (relativeTo == kSpaceSelf) {
		mQuat *= quat;
	}
	else {
		Transform3fAffine t(GetSRT());
		Eigen::Matrix4f l2w = GetLocalToWorldMatrix(); t *= l2w;
		t.rotate(quat);
		Eigen::Matrix4f w2l = GetWorldToLocalMatrix(); t *= w2l;

		mPosition = t.translation();
		mQuat = Eigen::Quaternionf(t.rotation());
	}
	mSignal();
}

TransformPtr Transform::GetRoot() const
{
	TransformPtr root = const_cast<Transform*>(this)->shared_from_this();
	while (root->GetParent())
		root = root->GetParent();
	return root;
}
TransformPtr Transform::GetChild(size_t index) const
{
	return index < mChildren.size() ? mChildren[index] : nullptr;
}

Eigen::Matrix4f Transform::GetLocalToWorldMatrix() const
{
	Eigen::Matrix4f l2w = Eigen::Matrix4f::Identity();
	for (TransformPtr parent = this->GetParent(); parent != nullptr; parent = parent->GetParent()) {
		l2w = l2w * parent->GetSRT();
	}
	return l2w;
}
Eigen::Matrix4f Transform::GetWorldToLocalMatrix() const
{
	return GetLocalToWorldMatrix().inverse();
}
Eigen::Matrix4f Transform::GetWorldMatrix() const
{
	return GetSRT() * GetLocalToWorldMatrix();
}

Eigen::Vector3f Transform::GetPosition() const
{
	Eigen::Matrix4f l2w = GetWorldMatrix();
	Transform3fAffine t(l2w);
	return t.translation();
}
Eigen::Vector3f Transform::GetLossyScale() const
{
	Eigen::Matrix4f l2w = GetWorldMatrix();
	Transform3fAffine t(l2w);
	return t.affine().diagonal().head<3>();
}
Eigen::Quaternionf Transform::GetRotation() const
{
	Eigen::Matrix4f l2w = GetWorldMatrix();
	Transform3fAffine t(l2w);
	return Eigen::Quaternionf(t.rotation());
}

Eigen::Vector3f Transform::GetForward() const
{
	auto forward = GetRotation() * math::vec::Forward();
	BOOST_ASSERT(fabs(forward.norm() - 1.0) < 0.01);
	return forward;
}
Eigen::Vector3f Transform::GetRight() const
{
	auto right = GetRotation() * math::vec::Right();
	BOOST_ASSERT(fabs(right.norm() - 1.0) < 0.01);
	return right;
}
Eigen::Vector3f Transform::GetUp() const
{
	auto up = GetRotation() * math::vec::Up();
	BOOST_ASSERT(fabs(up.norm() - 1.0) < 0.01);
	return up;
}
Eigen::Vector3f Transform::GetLookAt() const
{
	return this->GetLocalPosition() + this->GetForward() * mForwardLength;
}

Eigen::Vector3f Transform::TransformPoint(const Eigen::Vector3f& position) const
{
	Eigen::Matrix4f l2w = GetLocalToWorldMatrix();
	Transform3fAffine t(l2w);
	return t * position;
}
Eigen::Vector3f Transform::TransformVector(const Eigen::Vector3f& vector) const
{
	Eigen::Matrix4f l2w = GetLocalToWorldMatrix();
	Transform3fAffine t(l2w);
	Transform3fAffine tt(t.linear());
	return tt * vector;
}
Eigen::Vector3f Transform::TransformDirection(const Eigen::Vector3f& direction) const
{
	Eigen::Vector3f vec = TransformVector(direction);
	vec.normalize();
	return vec;
}

Eigen::Vector3f Transform::InverseTransformPoint(const Eigen::Vector3f& position) const
{
	Eigen::Matrix4f w2l = GetWorldToLocalMatrix();
	Transform3fAffine t(w2l);
	return t * position;
}
Eigen::Vector3f Transform::InverseTransformVector(const Eigen::Vector3f& vector) const
{
	Eigen::Matrix4f l2w = GetWorldToLocalMatrix();
	Transform3fAffine t(l2w);
	Transform3fAffine tt(t.linear());
	return tt * vector;
}
Eigen::Vector3f Transform::InverseTransformDirection(const Eigen::Vector3f& direction) const
{
	Eigen::Vector3f vec = InverseTransformVector(direction);
	vec.normalize();
	return vec;
}

}