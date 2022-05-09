#include <boost/math/constants/constants.hpp>
#include "core/resource/resource_manager.h"
#include "core/renderable/post_process.h"
#include "core/scene/camera.h"
#include "core/base/debug.h"
#include "core/base/math.h"
#include "test/unit_test/unit_test.h"

namespace mir {
namespace scene {

Camera::Camera(ResourceManager& resMng)
: mResMng(resMng)
{
	mTransform = CreateInstance<Transform>();
	mRenderPath = kRenderPathForward;
	mCameraMask = -1;
	mDepth = 0;

	mType = kCameraPerspective;
	mClipPlane = Eigen::Vector2f::Zero();
	mFov = mOrthoSize = 0;
	mAspect = 1.0;

	mViewDirty = true;
	mProjectionDirty = true;
}

void Camera::SetType(CameraType type)
{
	mType = type;
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
	mTransform->SetPosition(eye);
	mTransform->LookAt(at);
	mViewDirty = true;
}
void Camera::SetForward(const Eigen::Vector3f& forward)
{
	mTransform->LookForward(forward);
	mViewDirty = true;
}

CoTask<void> Camera::UpdateFrame(float dt)
{
	for (auto& effect : mPostProcessEffects)
		CoAwait effect->UpdateFrame(dt);
}

/********** query **********/
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
		mProjection = math::cam::MakePerspectiveFovLH(mFov, mAspect, mClipPlane[0], mClipPlane[1]);
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
	auto eyePos = mTransform->GetLocalPosition();
	auto forward = mTransform->GetForward();
	auto up = mTransform->GetUp();
	mView = math::cam::MakeLookForwardLH(eyePos, forward, up);

	Transform3fAffine t(Transform3fAffine::Identity());
	{
		auto s = mTransform->GetLocalScale();
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

math::Frustum Camera::GetFrustum() const
{
	math::Frustum ft;
	float tan_hfov = tanf(mFov * 0.5f);
	float n  = mClipPlane[0];
	float nh = tan_hfov * n;
	float nw = nh * mAspect;
	ft.FarPlane[0] = Eigen::Vector3f(-nw,-nh,n);
	ft.FarPlane[1] = Eigen::Vector3f( nw,-nh,n);
	ft.FarPlane[2] = Eigen::Vector3f( nw, nh,n);
	ft.FarPlane[3] = Eigen::Vector3f(-nw, nh,n);

	float f  = mClipPlane[1];
	float fh = tan_hfov * f;
	float fw = fh * mAspect;
	ft.FarPlane[0] = Eigen::Vector3f(-nw, -nh, f);
	ft.FarPlane[1] = Eigen::Vector3f(nw, -nh, f);
	ft.FarPlane[2] = Eigen::Vector3f(nw, nh, f);
	ft.FarPlane[3] = Eigen::Vector3f(-nw, nh, f);

	return ft;
}

/********** components **********/
const TransformPtr& Camera::GetTransform() const
{
	mViewDirty = true;
	return mTransform;
}
void Camera::SetSkyBox(const rend::SkyBoxPtr& skybox)
{
	mSkyBox = skybox;
}

void Camera::AddPostProcessEffect(const rend::PostProcessPtr& postEffect)
{
	if (mPostProcessEffects.empty() && mPostProcessInput == NULL)
		FetchOutput2PostProcess();

	mPostProcessEffects.push_back(postEffect);
}
IFrameBufferPtr Camera::FetchOutput2PostProcess(std::vector<ResourceFormat> formats)
{
	if (mPostProcessInput == nullptr) {
		mPostProcessInput = mResMng.CreateFrameBuffer(__LaunchSync__, Eigen::Vector3i(mResMng.WinWidth(), mResMng.WinHeight(), 1), formats);
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
	return SetOutput(mResMng.CreateFrameBuffer(__LaunchSync__, 
		Eigen::Vector3i(mResMng.WinWidth() * scale, mResMng.WinHeight() * scale, 1), formats));
}

/********** CameraFactory **********/
CameraPtr CameraFactory::CreatePerspective(float aspect, 
	const Eigen::Vector3f& eyePos, 
	const Eigen::Vector3f& length_forward, 
	const Eigen::Vector2f& clipPlane, 
	float fov, 
	unsigned cameraMask)
{
	CameraPtr camera = CreateInstance<Camera>(mResMng);

	camera->SetType(kCameraPerspective);
	camera->SetClippingPlane(clipPlane);
	camera->SetFov(fov);
	camera->SetAspect(aspect);
	camera->SetLookAt(eyePos, eyePos + length_forward);
	camera->SetCullingMask(cameraMask);

	return camera;
}

CameraPtr CameraFactory::CreateDefPerspective(const Eigen::Vector3f& eyePos)
{
	auto size = mResMng.WinSize();
	return CreatePerspective(1.0f * size.x() / size.y(), eyePos, math::vec::Forward() * fabs(eyePos.z()),
		math::cam::DefClippingPlane(), math::cam::DefFov());
}

CameraPtr CameraFactory::CreateOthogonal(float aspect, 
	const Eigen::Vector3f& eyePos, 
	const Eigen::Vector3f& length_forward, 
	const Eigen::Vector2f& clipPlane,
	float othoSize, 
	unsigned cameraMask)
{
	CameraPtr camera = CreateInstance<Camera>(mResMng);

	camera->SetType(kCameraOthogonal);
	camera->SetClippingPlane(clipPlane);
	camera->SetOrthographicSize(othoSize);
	camera->SetAspect(aspect);
	camera->SetLookAt(eyePos, eyePos + length_forward);
	camera->SetCullingMask(cameraMask);
	
	return camera;
}

CameraPtr CameraFactory::CreateDefOthogonal(const Eigen::Vector3f& eyePos)
{
	auto size = mResMng.WinSize();
	return CreateOthogonal(1.0f * size.x() / size.y(), eyePos, math::vec::Forward() * fabs(eyePos.z()),
		math::cam::DefClippingPlane(), math::cam::DefOthoSize() * mPixelPerUnit);
}

}
}