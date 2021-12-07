#include <boost/math/constants/constants.hpp>
#include "core/scene/camera.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/math.h"

namespace mir {

#define CAMERA_CENTER_IS_ZERO

CameraPtr Camera::CreatePerspective(ResourceManager& resMng, const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, double zfar, double fov)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
	camera->InitAsPerspective(screensize, eyePos, zfar, fov);
	return camera;
}
CameraPtr Camera::CreateOthogonal(ResourceManager& resMng, const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, double zfar)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
	camera->InitAsOthogonal(screensize, eyePos, zfar);
	return camera;
}

Camera::Camera(ResourceManager& resMng)
	: mResourceMng(resMng)
{
	mTransformOrViewDirty = true;
	mProjectionDirty = true;
	mFlipY = false;
	
	mType = kCameraPerspective;

	mView = Eigen::Matrix4f::Identity();
	mProjection = Eigen::Matrix4f::Identity();

	mTransform = std::make_shared<Transform>();
	mUpVector = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
}
void Camera::InitAsPerspective(const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, double zfar, double fov) 
{
	mType = kCameraPerspective;

	mSize = mScreenSize = screensize;
	mZFar = zfar;
	mFov = fov / 180.0 * boost::math::constants::pi<float>();

	SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
#if !defined CAMERA_CENTER_IS_ZERO
	mTransform->SetPosition(Eigen::Vector3f(mScreenSize.x() / 2, mScreenSize.y() / 2, 0));
	mTransformOrViewDirty = true;
#endif
	mProjectionDirty = true;
}
void Camera::InitAsOthogonal(const Eigen::Vector2i& screensize, const Eigen::Vector3f& eyePos, double zfar) 
{
	mType = kCameraOthogonal;

	mSize = mScreenSize = screensize;
	mZFar = zfar;

	SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
#if !defined CAMERA_CENTER_IS_ZERO
	mTransform->SetPosition(Eigen::Vector3f(mScreenSize.x() / 2, mScreenSize.y() / 2, 0));
	mTransformOrViewDirty = true;
#endif
	mProjectionDirty = true;
}

void Camera::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at)
{
	mEyePos = eye;
	mLookAtPos = at;
	mView = math::MakeLookAtLH(mEyePos, mLookAtPos, mUpVector);
	mTransformOrViewDirty = true;
}

void Camera::SetYFlipped(bool flip)
{
	mFlipY = flip;
	mProjectionDirty = true;
}

void Camera::SetSize(const Eigen::Vector2i& size)
{
	mSize = size;
	mProjectionDirty = true;
}

void Camera::RecalculateProjection() const
{
	switch (mType) {
	case kCameraPerspective:
		mProjection = math::MakePerspectiveFovLH(mFov, mScreenSize.x() * 1.0 / mScreenSize.y(), 0.01f, mZFar);
		break;
	case kCameraOthogonal:
		mProjection = math::MakeOrthographicOffCenterLH(0, mScreenSize.x(), 0, mScreenSize.y(), 0.01, mZFar);
		break;
	default:
		break;
	}

	if (mFlipY) {
		mProjection = Transform3Projective(mProjection)
			.prescale(Eigen::Vector3f(1, -1, 1))
			.matrix();
	}
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
#elif 1
		Eigen::Vector4f lb = Transform3Projective(mProjection) * Eigen::Vector4f(0, 0, 0, 1);
		Eigen::Vector4f rt = Transform3Projective(mProjection) * Eigen::Vector4f(1024, 768, 0, 1);
		mProjection = mProjection;
#endif
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

const Eigen::Matrix4f& Camera::GetView() const
{
	if (mTransformOrViewDirty) {
		mTransformOrViewDirty = false;
	#if !defined CAMERA_CENTER_IS_ZERO
		const auto& srt = mTransform->SetMatrixSRT();
		Transform3fAffine t(srt.inverse());//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]

		t.pretranslate(mTransform->GetPosition());//[-screen.hw, screen.hw] -> [0->sreen.w]

		mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
	#else
		Eigen::Vector3f center(mScreenSize.x() / 2, mScreenSize.y() / 2, 0);
		Transform3fAffine t = Transform3fAffine::Identity();
		t.pretranslate(-center);//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]

		const auto& srt = mTransform->SetMatrixSRT();
		t = Transform3fAffine(srt.inverse() * t.matrix());

		t.pretranslate(center + mTransform->GetPosition());//[-screen.hw, screen.hw] -> [0->sreen.w]

		mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
		mWorldView = Eigen::Matrix4f::Identity();
	#endif
	}
	return mWorldView;
}

/********** components **********/
const TransformPtr& Camera::GetTransform() const
{
	mTransformOrViewDirty = true;
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
IFrameBufferPtr Camera::FetchOutput(ResourceFormat format, ResourceFormat zstencilFmt)
{
	if (mOutput == nullptr) {
		mSize = mScreenSize / 4;
		mOutput = mResourceMng.CreateFrameBuffer(__LaunchSync__, mSize, MakeResFormats(format, zstencilFmt));
		DEBUG_SET_PRIV_DATA(mOutput, "camera.output");
	}
	return mOutput;
}

/********** query **********/
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

}