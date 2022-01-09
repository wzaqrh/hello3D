#include <boost/math/constants/constants.hpp>
#include "core/scene/camera.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/math.h"
#include "test/unit_test/unit_test.h"

namespace mir {

Camera::Camera(ResourceManager& resMng)
	: mResourceMng(resMng)
{
	mTransform = CreateInstance<Transform>();
	mRenderPath = kRenderPathForward;
	mCameraMask = -1;
	mDepth = 0;

	mType = kCameraPerspective;
	mClipPlane = Eigen::Vector2f::Zero();
	mFov = mOrthoSize = 0;
	mAspect = mForwardLength = 1.0;

	mViewDirty = true;
	mProjectionDirty = true;
}

void Camera::InitAsPerspective(float aspect,
	const Eigen::Vector3f& eyePos, 
	const Eigen::Vector3f& length_forward, 
	const Eigen::Vector2f& clipPlane,
	float fov,
	unsigned cameraMask) 
{
	mType = kCameraPerspective;
	SetClippingPlane(clipPlane);
	SetFov(fov);
	SetAspect(aspect);
	SetLookAt(eyePos, eyePos + length_forward);
	SetCullingMask(cameraMask);
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
	SetClippingPlane(clipPlane);
	SetOrthographicSize(othoSize);
	SetAspect(aspect);
	SetLookAt(eyePos, eyePos + length_forward);
	SetCullingMask(cameraMask);
	mProjectionDirty = true;
}

void Camera::SetClippingPlane(const Eigen::Vector2f& zRange)
{
	mClipPlane = zRange;
	mProjectionDirty = true;
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
void Camera::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at)
{
	auto forward = at - eye;
	mForwardLength = forward.norm();
	mTransform->SetRotation(Eigen::Quaternionf::FromTwoVectors(math::vec::Forward(), forward));
	mTransform->SetPosition(eye);
	mViewDirty = true;
}
void Camera::SetForward(const Eigen::Vector3f& forward)
{
	mTransform->SetRotation(Eigen::Quaternionf::FromTwoVectors(math::vec::Forward(), forward));
	mViewDirty = true;
}

/********** query **********/
Eigen::Vector3f Camera::GetLookAt() const
{
	return mTransform->GetPosition() + mTransform->GetForward() * mForwardLength;
}
Eigen::Vector3f Camera::GetForward() const
{
	return mTransform->GetForward();
}
Eigen::Vector3f Camera::GetUp() const
{
	return mTransform->GetUp();
}
Eigen::Vector2f Camera::GetOthoWinSize() const
{
	BOOST_ASSERT(mType == kCameraOthogonal);
	float winHeight = mOrthoSize * 2;
	return Eigen::Vector2f(winHeight * mAspect, winHeight);
}

//#define OTHO_PROJ_CENTER_NOT_ZERO
void Camera::RecalculateProjection() const
{
	switch (mType) {
	case kCameraPerspective:
		mProjection = math::cam::MakePerspectiveFovLH(mFov, mAspect, mClipPlane.x(), mClipPlane.y());
		break;
	case kCameraOthogonal: {
	#if defined OTHO_PROJ_CENTER_NOT_ZERO
		Eigen::Vector2f othoWin = GetOthoWinSize();
		mProjection = math::cam::MakeOrthographicOffCenterLH(0, othoWin.x(),
			0, othoWin.y(),
			mClipPlane.x(), mClipPlane.y());

		auto lb = Transform3Projective(mProjection) * Eigen::Vector4f(0, 0, 0, 1); lb /= lb.w();
		BOOST_ASSERT(test::IsEqual(lb.head<2>(), Eigen::Vector2f(-1, -1)));
		auto rt = Transform3Projective(mProjection) * Eigen::Vector4f(othoWin.x(), othoWin.y(), 0, 1); rt /= rt.w();
		BOOST_ASSERT(test::IsEqual(rt.head<2>(), Eigen::Vector2f(1, 1)));
	#else
		Eigen::Vector2f othoWin = GetOthoWinSize() / 2;
		mProjection = math::cam::MakeOrthographicOffCenterLH(-othoWin.x(), othoWin.x(),
			-othoWin.y(), othoWin.y(),
			mClipPlane.x(), mClipPlane.y());

		/*auto lb = Transform3Projective(mProjection) * Eigen::Vector4f(-othoWin.x(), -othoWin.y(), 0, 1); lb /= lb.w();
		BOOST_ASSERT(test::IsEqual(lb.head<2>(), Eigen::Vector2f(-1, -1)));
		auto rt = Transform3Projective(mProjection) * Eigen::Vector4f(othoWin.x(), othoWin.y(), 0,1); rt /= rt.w();
		BOOST_ASSERT(test::IsEqual(rt.head<2>(), Eigen::Vector2f(1, 1)));*/
	#endif
	}break;
	default:
		break;
	}

#if 0
	if (mFlipY) {
		mProjection = Transform3Projective(mProjection)
			.prescale(Eigen::Vector3f(1, -1, 1))
			.matrix();
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
	auto eyePos = mTransform->GetPosition();
	auto forward = mTransform->GetForward();
	auto up = mTransform->GetUp();
	mView = math::cam::MakeLookForwardLH(eyePos, forward, up);

	Transform3fAffine t(Transform3fAffine::Identity());
	{
		auto s = mTransform->GetScale();
		t.prescale(Eigen::Vector3f(1 / s.x(), 1 / s.y(), 1 / s.z()));

	#if defined OTHO_PROJ_CENTER_NOT_ZERO
		if (mType == kCameraOthogonal) {
			Eigen::Vector2f othoWin = GetOthoWinSize();
			Eigen::Vector3f center(othoWin.x() / 2, othoWin.y() / 2, 0);
			t.pretranslate(center);//[-screen.hw, screen.hw] -> [0->sreen.w]
		}
	#endif
	}
	mWorldView = t.matrix() * mView;

	/*auto lb = Transform3Projective(mWorldView) * Eigen::Vector4f(-5, -5, 0, 1); lb /= lb.w();
	auto rt = Transform3Projective(mWorldView) * Eigen::Vector4f(5, 5, 0, 1); rt /= rt.w();*/
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