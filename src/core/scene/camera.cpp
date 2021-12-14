#include <boost/math/constants/constants.hpp>
#include "core/scene/camera.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/math.h"

namespace mir {

//#define OTHO_CAMERA_TRASNFORM_CENTER_MAPTO_SCREEN_CENTER

Camera::Camera(ResourceManager& resMng)
	: mResourceMng(resMng)
{
	mViewDirty = true;
	mProjectionDirty = true;
	mFlipY = false;
	mType = kCameraPerspective;

	mAspect = 1.0;
	mTransform = std::make_shared<Transform>();
}

void Camera::InitAsPerspective(float aspect,
	const Eigen::Vector3f& eyePos, 
	const Eigen::Vector3f& length_forward, 
	const Eigen::Vector2f& clipPlane,
	float fov,
	unsigned cameraMask) 
{
	mType = kCameraPerspective;
	//mSize = mScreenSize = screensize;
	SetClippingPlane(clipPlane);
	SetFov(fov);
	SetAspect(aspect);
	SetLookAt(eyePos, eyePos + length_forward, math::vec::Up());
	SetCameraMask(cameraMask);
	mProjectionDirty = true;
}
void Camera::InitAsOthogonal(float aspect,
	const Eigen::Vector3f& eyePos, 
	const Eigen::Vector3f& length_forward, 
	const Eigen::Vector2f& clipPlane,
	float othoSize,
	unsigned cameraMask)
{
	mType = kCameraOthogonal;
	//mSize = mScreenSize = screensize;
	SetClippingPlane(clipPlane);
	SetOrthographicSize(othoSize);
	SetAspect(aspect);
	SetLookAt(eyePos, eyePos + length_forward, math::vec::Up());
	SetCameraMask(cameraMask);
	mProjectionDirty = true;
}

void Camera::SetClippingPlane(const Eigen::Vector2f& zRange)
{
	mZRange = zRange;
	mViewDirty = true;
}

void Camera::SetAspect(float aspect)
{
	mAspect = aspect;
	mProjectionDirty = mViewDirty = true;
}

void Camera::SetFov(float fov)
{
	mFov = fov / boost::math::constants::radian<float>();
	mProjectionDirty = true;
}
void Camera::SetOrthographicSize(float size)
{
	mOrthoSize = size;
	mProjectionDirty = true;
}
void Camera::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at, const Eigen::Vector3f& up)
{
	mTransform->SetPosition(eye);
	auto forward = at - eye;
	mTransform->SetRotation(Eigen::Quaternionf::FromTwoVectors(math::vec::Forward(), forward));
	mForwardLength = forward.norm();

	mUpVector = up;
	mViewDirty = true;
}

void Camera::SetForwardLength(float length)
{
	mForwardLength = length;
	mViewDirty = true;
}

void Camera::SetYFlipped(bool flip)
{
	mFlipY = flip;
	mProjectionDirty = true;
}

/********** query **********/
Eigen::Vector3f Camera::GetEye() const 
{
	return mTransform->GetPosition();
}
Eigen::Vector3f Camera::GetLookAt() const 
{
	return mTransform->GetPosition() + mTransform->GetForward() * mForwardLength;
}
Eigen::Vector3f Camera::GetForward() const 
{
	return mTransform->GetForward();
}
float Camera::GetForwardLength() const 
{
	return mForwardLength;
}

Eigen::Vector2f Camera::GetOthoWinSize() const
{
	float winHeight = mOrthoSize * 2;
	return Eigen::Vector2f(winHeight * mAspect, winHeight);
}

void Camera::RecalculateProjection() const
{
	switch (mType) {
	case kCameraPerspective:
		mProjection = math::MakePerspectiveFovLH(mFov, mAspect, mZRange.x(), mZRange.y());
		break;
	case kCameraOthogonal: {
		Eigen::Vector2f othoWin = GetOthoWinSize();
		mProjection = math::MakeOrthographicOffCenterLH(0, othoWin.x(),
			0, othoWin.y(),
			mZRange.x(), mZRange.y());
	}break;
	default:
		break;
	}

	if (mFlipY) {
		mProjection = Transform3Projective(mProjection)
			.prescale(Eigen::Vector3f(1, -1, 1))
			.matrix();
	}
}
const Eigen::Matrix4f& Camera::GetProjection() const
{
	if (mProjectionDirty) {
		mProjectionDirty = false;
		RecalculateProjection();
	}
	return mProjection;
}

void Camera::RecalculateView() const
{
	auto eyePos = mTransform->GetPosition();
	auto forward = mTransform->GetForward();
	mView = math::MakeLookForwardLH(eyePos, forward, mUpVector);

	Transform3fAffine t(Transform3fAffine::Identity());
	{
		Eigen::Vector2f othoWin = GetOthoWinSize();
		Eigen::Vector3f center(othoWin.x() / 2, othoWin.y() / 2, 0);
		if (mType == kCameraOthogonal) {
			t.translate(center);//consider orho-camera's anchor is screen-center
			t.pretranslate(-center);//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]
		}

		auto s = mTransform->GetScale();
		t.prescale(Eigen::Vector3f(1 / s.x(), 1 / s.y(), 1 / s.z()));

		if (mType == kCameraOthogonal)
			t.pretranslate(center);//[-screen.hw, screen.hw] -> [0->sreen.w]
	}
	mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
}
const Eigen::Matrix4f& Camera::GetView() const
{
	if (mViewDirty) {
		mViewDirty = false;
		RecalculateView();
	}
	return mWorldView;
}

Eigen::Vector4f Camera::ProjectPoint(const Eigen::Vector4f& pos) const
{
	Eigen::Vector4f perspective = Transform3Projective(GetProjection() * GetView()) * pos;

	if (perspective.w() != 0) {
		perspective.head<3>() /= perspective.w();
		perspective.w() = 1;
	}
	else {
		perspective = Eigen::Vector4f(0, 0, 0, 0);
	}
	return perspective;
}
Eigen::Vector3f Camera::ProjectPoint(const Eigen::Vector3f& pos) const
{
	return ProjectPoint(Eigen::Vector4f(pos.x(), pos.y(), pos.z(), 1.0)).head<3>();
}

/********** components **********/
const TransformPtr& Camera::GetTransform() const
{
	mViewDirty = true;
	return mTransform;
}
void Camera::SetSkyBox(const SkyBoxPtr& skybox)
{
	mSkyBox = skybox;
}

void Camera::AddPostProcessEffect(const PostProcessPtr& postEffect)
{
	mPostProcessEffects.push_back(postEffect);
}
IFrameBufferPtr Camera::FetchOutput2PostProcess(ResourceFormat format)
{
	if (mPostProcessInput == nullptr) {
		mPostProcessInput = mResourceMng.CreateFrameBuffer(__LaunchSync__, mResourceMng.WinSize(), format);
		DEBUG_SET_PRIV_DATA(mPostProcessInput, "camera.output_to_post_process");
	}
	return mPostProcessInput;
}

IFrameBufferPtr Camera::SetOutput(IFrameBufferPtr output)
{
	mOutput = output;
	DEBUG_SET_PRIV_DATA(mOutput, "camera.output");

	//mSize = mOutput->GetSize();
	mProjectionDirty = true;

	return mOutput;
}
IFrameBufferPtr Camera::SetOutput(float scale, std::vector<ResourceFormat> formats)
{
	return SetOutput(mResourceMng.CreateFrameBuffer(__LaunchSync__, (mResourceMng.WinSize().cast<float>() * scale).cast<int>(), formats));
}

}