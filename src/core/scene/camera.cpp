#include <boost/math/constants/constants.hpp>
#include "core/scene/camera.h"
//#include "core/rendersys/render_pipeline.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/math.h"

namespace mir {

CameraPtr Camera::CreatePerspective(ResourceManager& resMng, const Eigen::Vector2i& size, 
	Eigen::Vector3f eyePos, double far1, double fov)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
	camera->SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
	camera->SetPerspectiveProj(size, fov, far1);
	return camera;
}

CameraPtr Camera::CreateOthogonal(ResourceManager& resMng, const Eigen::Vector2i& size, 
	Eigen::Vector3f eyePos, double far1)
{
	CameraPtr camera = std::make_shared<Camera>(resMng);
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
		SetPerspectiveProj(mSize, mFov, mZFar);
		break;
	case kCameraOthogonal:
		SetOthogonalProj(mSize, mZFar);
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

	mTransform->SetPosition(Eigen::Vector3f(mSize.x() / 2, mSize.y() / 2, 0));
	mTransformDirty = true;
}

void Camera::SetOthogonalProj(const Eigen::Vector2i& size, double zFar)
{
	mType = kCameraOthogonal;

	mSize = size;
	mZFar = zFar;
	mProjection = math::MakeOrthographicOffCenterLH(0, mSize.x(), 0, mSize.y(), 0.01, mZFar);

	if (mFlipY) mProjection = Transform3Projective(mProjection).scale(Eigen::Vector3f(1, -1, 1)).matrix();

	mTransform->SetPosition(Eigen::Vector3f(mSize.x() / 2, mSize.y() / 2, 0));
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

		auto position = mTransform->GetPosition();
		{
			auto newpos = position;
			auto scale = mTransform->GetScale();
			newpos.x() = position.x() - scale.x() * mSize.x() / 2;
			newpos.y() = position.y() - scale.y() * mSize.y() / 2;
			newpos.z() = position.z();
			mTransform->SetPosition(newpos);

			const auto& srt = mTransform->SetMatrixSRT();
			mWorldView = mView * srt.inverse();
		}
		mTransform->SetPosition(position);
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

IFrameBufferPtr Camera::FetchPostProcessInput(ResourceFormat format)
{
	if (mPostProcessInput == nullptr) {
		mPostProcessInput = mResourceMng.CreateFrameBuffer(__LaunchSync__, mSize, format);
		//SET_DEBUG_NAME(mPostProcessInput->mDepthStencilView, "post_process_input");
	}
	return mPostProcessInput;
}

IFrameBufferPtr Camera::FetchOutput(ResourceFormat format)
{
	if (mOutput == nullptr) {
		mOutput = mResourceMng.CreateFrameBuffer(__LaunchSync__, mSize, format);
	}
	return mOutput;
}

}