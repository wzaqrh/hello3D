#include <boost/math/constants/constants.hpp>
#include "core/scene/camera.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/math.h"

namespace mir {

#define CAMERA_CENTER_IS_ZERO

CameraPtr Camera::CreatePerspective(ResourceManager& resMng, const Eigen::Vector3f& eyePos, float zfar, float fov)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
	camera->InitAsPerspective(resMng.WinSize(), eyePos, zfar, fov);
	return camera;
}
CameraPtr Camera::CreateOthogonal(ResourceManager& resMng, const Eigen::Vector3f& eyePos, float zfar)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
	camera->InitAsOthogonal(resMng.WinSize(), eyePos, zfar);
	return camera;
}

Camera::Camera(ResourceManager& resMng)
	: mResourceMng(resMng)
{
	mViewDirty = true;
	mProjectionDirty = true;
	mFlipY = false;
	mType = kCameraPerspective;

	mTransform = std::make_shared<Transform>();
}

void Camera::InitAsPerspective(const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, float zfar, float fov) 
{
	mType = kCameraPerspective;
	mSize = mScreenSize = screensize;
	SetZRange(Eigen::Vector2f(0.01, zfar));
	SetFov(fov);
	SetLookAt(eyePos, math::vec::Zero(), math::vec::Up());
#if !defined CAMERA_CENTER_IS_ZERO
	mTransform->SetPosition(Eigen::Vector3f(mScreenSize.x() / 2, mScreenSize.y() / 2, 0));
	mViewDirty = true;
#endif
	mProjectionDirty = true;
}
void Camera::InitAsOthogonal(const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, float zfar) 
{
	mType = kCameraOthogonal;
	mSize = mScreenSize = screensize;
	SetZRange(Eigen::Vector2f(0.01, zfar));
	SetLookAt(eyePos, math::vec::Zero(), math::vec::Up());
#if !defined CAMERA_CENTER_IS_ZERO
	mTransform->SetPosition(Eigen::Vector3f(mScreenSize.x() / 2, mScreenSize.y() / 2, 0));
	mViewDirty = true;
#endif
	mProjectionDirty = true;
}

void Camera::SetZRange(const Eigen::Vector2f& zRange)
{
	mZRange = zRange;
	mViewDirty = true;
}
void Camera::SetFov(float fov)
{
	mFov = fov / 180.0 * boost::math::constants::pi<float>();
	mViewDirty = true;
}
void Camera::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at, const Eigen::Vector3f& up)
{
#if defined CAMERA_TRANSFORM
	mTransform->SetPosition(eye);
	auto forward = at - eye;
	mTransform->SetQuaternion(Eigen::Quaternionf::FromTwoVectors(math::vec::Forward(), forward));
	mForwardLength = forward.norm();
#else
	mEyePos = eye;
	mForwardVector = at - eye;
#endif
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
#if defined CAMERA_TRANSFORM
Eigen::Vector3f Camera::GetEye() const {
	return mTransform->GetPosition();
}
Eigen::Vector3f Camera::GetLookAt() const {
	return mTransform->GetPosition() + mTransform->GetForward() * mForwardLength;
}
Eigen::Vector3f Camera::GetForward() const {
	return mTransform->GetForward();
}
float Camera::GetForwardLength() const {
	return mForwardLength;
}
#else
Eigen::Vector3f Camera::GetEye() const { 
	return mEyePos; 
}
Eigen::Vector3f Camera::GetLookAt() const { 
	return mEyePos + mForwardVector; 
}
Eigen::Vector3f Camera::GetForward() const {
	return mForwardVector.normalized();
}
float Camera::GetForwardLength() const {
	return mForwardVector.norm();
}
#endif

void Camera::RecalculateProjection() const
{
	switch (mType) {
	case kCameraPerspective:
		mProjection = math::MakePerspectiveFovLH(mFov, mScreenSize.x() * 1.0 / mScreenSize.y(), mZRange.x(), mZRange.y());
		break;
	case kCameraOthogonal:
		mProjection = math::MakeOrthographicOffCenterLH(0, mScreenSize.x(), 0, mScreenSize.y(), mZRange.x(), mZRange.y());
		break;
	default:
		break;
	}

	if (mFlipY) {
		mProjection = Transform3Projective(mProjection)
			.prescale(Eigen::Vector3f(1, -1, 1))
			.matrix();
	}
#if 0
	if (mScreenSize != mSize) {
		float w2 = 1.0f * mSize.x() / mScreenSize.x();
		float h2 = 1.0f * mSize.y() / mScreenSize.y();
		mProjection = Transform3Projective(mProjection)
			.pretranslate(Eigen::Vector3f(1, 1, 0))//[(-1,-1), (1,1)] => [(0,0), (2,2)]
			.prescale(Eigen::Vector3f(w2, h2, 1))//[(0,0), (2,2)] => [(0,0), (w,h)]
			.pretranslate(Eigen::Vector3f(-1, -1, 0))//[(0,0), (w,h)] => [(-1,-1), (w-1,h-1)]
			.pretranslate(Eigen::Vector3f(0, 2 - h2*2, 0))//[(-1,-1), (w-1,h-1)] => 若rt比屏幕小, 则rt位置范围[(-1,1-h), (-1+w,1)]
			.matrix();
	#if 0
		Transform3Projective t = Transform3Projective::Identity();
		t.pretranslate(Eigen::Vector3f(1, 1, 0))
		.prescale(Eigen::Vector3f(1.0f * mSize.x() / mScreenSize.x(), 1.0f * mSize.y() / mScreenSize.y(), 1))
		.pretranslate(Eigen::Vector3f(-1, -1, 0));
		Eigen::Vector4f lb = t * Eigen::Vector4f(-1, -1, 0, 1);
		Eigen::Vector4f rt = t * Eigen::Vector4f(1, 1, 0, 1);
		mProjection = mProjection;
	#elif 0
		Eigen::Vector4f lb = Transform3Projective(mProjection) * Eigen::Vector4f(0, 0, 0, 1);
		Eigen::Vector4f rt = Transform3Projective(mProjection) * Eigen::Vector4f(1024, 768, 0, 1);
		mProjection = mProjection;
	#endif
	}
#endif
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
#if !defined CAMERA_CENTER_IS_ZERO
	const auto& srt = mTransform->SetMatrixSRT();
	Transform3fAffine t(srt.inverse());//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]

	t.pretranslate(mTransform->GetPosition());//[-screen.hw, screen.hw] -> [0->sreen.w]

	mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
#else
#if defined CAMERA_TRANSFORM
	auto eyePos = mTransform->GetPosition();
	auto forward = mTransform->GetForward();
	mView = math::MakeLookForwardLH(eyePos, forward, mUpVector);

	Transform3fAffine t(Transform3fAffine::Identity());
	{
		Eigen::Vector3f center(mScreenSize.x() / 2, mScreenSize.y() / 2, 0);
		if (mType == kCameraOthogonal) 
			t.pretranslate(-center);//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]

		auto s = mTransform->GetScale();
		t.prescale(Eigen::Vector3f(1 / s.x(), 1 / s.y(), 1 / s.z()));

		if (mType == kCameraOthogonal)
			t.pretranslate(center);//[-screen.hw, screen.hw] -> [0->sreen.w]
	}
	mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
#else
	mView = math::MakeLookForwardLH(mEyePos, mForwardVector, mUpVector);

	Eigen::Vector3f center(mScreenSize.x() / 2, mScreenSize.y() / 2, 0);
	Transform3fAffine t = Transform3fAffine::Identity();
	t.pretranslate(-center);//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]

	const auto& srt = mTransform->GetSRT();
	t = Transform3fAffine(srt.inverse() * t.matrix());

	t.pretranslate(center + mTransform->GetPosition());//[-screen.hw, screen.hw] -> [0->sreen.w]

	mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
#endif
#endif
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
	Eigen::Vector4f perspective = Transform3Projective(mProjection * GetView()) * pos;

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
		mPostProcessInput = mResourceMng.CreateFrameBuffer(__LaunchSync__, mScreenSize / 4, format);
		DEBUG_SET_PRIV_DATA(mPostProcessInput, "camera.output_to_post_process");
	}
	return mPostProcessInput;
}

IFrameBufferPtr Camera::SetOutput(IFrameBufferPtr output)
{
	mOutput = output;
	DEBUG_SET_PRIV_DATA(mOutput, "camera.output");

	mSize = mOutput->GetSize();
	mProjectionDirty = true;

	return mOutput;
}
IFrameBufferPtr Camera::SetOutput(float scale, std::vector<ResourceFormat> formats)
{
	return SetOutput(mResourceMng.CreateFrameBuffer(__LaunchSync__, (mScreenSize.cast<float>() * scale).cast<int>(), formats));
}

}