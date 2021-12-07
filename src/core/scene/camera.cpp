#include <boost/math/constants/constants.hpp>
#include "core/scene/camera.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/math.h"

namespace mir {

#define CAMERA_CENTER_IS_ZERO

CameraPtr Camera::CreatePerspective(ResourceManager& resMng, const Eigen::Vector2i& size, 
	Eigen::Vector3f eyePos, double far1, double fov)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
	camera->mScreenSize = size;
	camera->SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
	camera->SetPerspectiveProj(size, fov, far1);
	return camera;
}

CameraPtr Camera::CreateOthogonal(ResourceManager& resMng, const Eigen::Vector2i& size, 
	Eigen::Vector3f eyePos, double far1)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
	camera->mScreenSize = size;
	camera->SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
	camera->SetOthogonalProj(size, far1);
	return camera;
}

Camera::Camera(ResourceManager& resMng)
	: mResourceMng(resMng)
{
	mFlipY = false;
	mTransformDirty = true;
	mType = kCameraPerspective;

	mView = Eigen::Matrix4f::Identity();
	mProjection = Eigen::Matrix4f::Identity();

	mTransform = std::make_shared<Transform>();
	mUpVector = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
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

void Camera::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at)
{
	mEyePos = eye;
	mLookAtPos = at;
	mView = math::MakeLookAtLH(mEyePos, mLookAtPos, mUpVector);
	mTransformDirty = true;
}
void Camera::SetFlipY(bool flip)
{
	mFlipY = flip;
	switch (mType) {
	case kCameraPerspective:
		SetPerspectiveProj(mScreenSize, mFov, mZFar);
		break;
	case kCameraOthogonal:
		SetOthogonalProj(mScreenSize, mZFar);
		break;
	default:
		break;
	}
}

void Camera::SetPerspectiveProj(const Eigen::Vector2i& size, double fov, double zFar)
{
	mType = kCameraPerspective;

	mSize = size;
	mZFar = zFar;
	mFov = fov / 180.0 * boost::math::constants::pi<float>();
	mProjection = math::MakePerspectiveFovLH(mFov, mSize.x() * 1.0 / mSize.y(), 0.01f, mZFar);

	if (mFlipY) mProjection = Transform3Projective(mProjection).scale(Eigen::Vector3f(1, -1, 1)).matrix();

#if !defined CAMERA_CENTER_IS_ZERO
	mTransform->SetPosition(Eigen::Vector3f(mSize.x() / 2, mSize.y() / 2, 0));
#endif
	mTransformDirty = true;
}

void Camera::SetOthogonalProj(const Eigen::Vector2i& size, double zFar)
{
	mType = kCameraOthogonal;

	mSize = size;
	mZFar = zFar;
	mProjection = math::MakeOrthographicOffCenterLH(0, mSize.x(), 0, mSize.y(), 0.01, mZFar);

	if (mFlipY) mProjection = Transform3Projective(mProjection).scale(Eigen::Vector3f(1, -1, 1)).matrix();
#if !defined CAMERA_CENTER_IS_ZERO
	mTransform->SetPosition(Eigen::Vector3f(mSize.x() / 2, mSize.y() / 2, 0));
#endif
	mTransformDirty = true;
}

const TransformPtr& Camera::GetTransform() const
{
	mTransformDirty = true;
	return mTransform;
}

const Eigen::Matrix4f& Camera::GetView() const
{
	if (mTransformDirty) {
		mTransformDirty = false;
	#if !defined CAMERA_CENTER_IS_ZERO
		const auto& srt = mTransform->SetMatrixSRT();
		Transform3fAffine t(srt.inverse());//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]

		t.pretranslate(mTransform->GetPosition());//[-screen.hw, screen.hw] -> [0->sreen.w]

		mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
	#else
		Eigen::Vector3f center(mSize.x() / 2, mSize.y() / 2, 0);
		Transform3fAffine t = Transform3fAffine::Identity();
		t.pretranslate(-center);//[0->sreen.w]  -> relative2screen_center[-screen.hw, screen.hw]

		const auto& srt = mTransform->SetMatrixSRT();
		t = Transform3fAffine(srt.inverse() * t.matrix());

		t.pretranslate(center + mTransform->GetPosition());//[-screen.hw, screen.hw] -> [0->sreen.w]

		mWorldView = mView * t.matrix();//[0->sreen.w] -> view_space
	#endif
	}
	return mWorldView;
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
		mPostProcessInput = mResourceMng.CreateFrameBuffer(__LaunchSync__, mScreenSize, format);
		DEBUG_SET_PRIV_DATA(mPostProcessInput, "camera.output_to_post_process");
	}
	return mPostProcessInput;
}

IFrameBufferPtr Camera::FetchOutput(ResourceFormat format, ResourceFormat zstencilFmt)
{
	if (mOutput == nullptr) {
		mOutput = mResourceMng.CreateFrameBuffer(__LaunchSync__, mScreenSize, MakeResFormats(format,zstencilFmt));
		DEBUG_SET_PRIV_DATA(mOutput, "camera.output");
	}
	return mOutput;
}

}