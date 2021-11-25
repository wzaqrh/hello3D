#include <boost/math/constants/constants.hpp>
#include "core/scene/camera.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/render_pipeline.h"
#include "core/base/debug.h"
#include "core/base/math.h"

namespace mir {

CameraPtr Camera::CreatePerspective(RenderSystem& renderSys, const Eigen::Vector2i& size, 
	Eigen::Vector3f eyePos, double far1, double fov)
{
	CameraPtr camera = std::make_shared<Camera>(renderSys);
	camera->SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
	camera->SetPerspectiveProj(size, fov, far1);
	return camera;
}

CameraPtr Camera::CreateOthogonal(RenderSystem& renderSys, const Eigen::Vector2i& size, 
	Eigen::Vector3f eyePos, double far1)
{
	CameraPtr camera = std::make_shared<Camera>(renderSys);
	camera->SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
	camera->SetOthogonalProj(size, far1);
	return camera;
}

Camera::Camera(RenderSystem& renderSys)
	:mRenderSys(renderSys)
{
	mFlipY = false;
	mTransformDirty = true;
	mIsPespective = true;

	mView = Eigen::Matrix4f::Identity();
	mProjection = Eigen::Matrix4f::Identity();

	mTransform = std::make_shared<Transform>();
	mUpVector = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
}

Eigen::Vector4f Camera::ProjectPoint(const Eigen::Vector4f& pos) const
{
	Transform3fAffine t(mView * mProjection);
	Eigen::Vector4f perspective = t * pos;

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
	if (mIsPespective) SetPerspectiveProj(mSize, mFov, mZFar);
	else SetOthogonalProj(mSize, mZFar);
}

void Camera::SetPerspectiveProj(const Eigen::Vector2i& size, double fov, double zFar)
{
	mIsPespective = true;

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
	mIsPespective = false;

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
			mWorldView = srt.inverse() * mView;
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

IFrameBufferPtr Camera::FetchPostProcessInput()
{
	if (mPostProcessInput == nullptr) {
		mPostProcessInput = mRenderSys.LoadFrameBuffer(nullptr, mSize, kFormatR16G16B16A16UNorm);
		//SET_DEBUG_NAME(mPostProcessInput->mDepthStencilView, "post_process_input");
	}
	return mPostProcessInput;
}

}