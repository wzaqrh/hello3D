#include <boost/math/constants/constants.hpp>
#include "core/rendersys/camera.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/render_pipeline.h"
#include "core/base/debug.h"
#include "core/base/math.h"


namespace mir {

CameraPtr Camera::CreatePerspective(RenderSystem& renderSys, int width, int height, Eigen::Vector3f eyePos, double far1, double fov)
{
	CameraPtr pCam = std::make_shared<Camera>(renderSys);
	pCam->SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
	pCam->SetPerspectiveProj(width, height, fov, far1);
	return pCam;
}

CameraPtr Camera::CreateOthogonal(RenderSystem& renderSys, int width, int height, Eigen::Vector3f eyePos, double far1)
{
	CameraPtr pCam = std::make_shared<Camera>(renderSys);
	pCam->SetLookAt(eyePos, Eigen::Vector3f(0, 0, 0));
	pCam->SetOthogonalProj(width, height, far1);
	return pCam;
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
	if (mIsPespective) SetPerspectiveProj(mWidth, mHeight, mFov, mZFar);
	else SetOthogonalProj(mWidth, mHeight, mZFar);
}

void Camera::SetPerspectiveProj(int width, int height, double fov, double zFar)
{
	mIsPespective = true;

	mWidth = width;
	mHeight = height;
	mZFar = zFar;
	mFov = fov / 180.0 * boost::math::constants::pi<float>();
	mProjection = math::MakePerspectiveFovLH(mFov, mWidth * 1.0 / mHeight, 0.01f, mZFar);

	if (mFlipY) mProjection = Transform3Projective(mProjection).scale(Eigen::Vector3f(1, -1, 1)).matrix();

	mTransform->SetPosition(Eigen::Vector3f(mWidth / 2, mHeight / 2, 0));
	mTransformDirty = true;
}

void Camera::SetOthogonalProj(int width, int height, double zFar)
{
	mIsPespective = false;

	mWidth = width;
	mHeight = height;
	mZFar = zFar;
	mProjection = math::MakeOrthographicOffCenterLH(0, mWidth, 0, mHeight, 0.01, mZFar);

	if (mFlipY) mProjection = Transform3Projective(mProjection).scale(Eigen::Vector3f(1, -1, 1)).matrix();

	mTransform->SetPosition(Eigen::Vector3f(mWidth / 2, mHeight / 2, 0));
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
			newpos.x() = position.x() - scale.x() * mWidth / 2;
			newpos.y() = position.y() - scale.y() * mHeight / 2;
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

IRenderTexturePtr Camera::FetchPostProcessInput()
{
	if (mPostProcessInput == nullptr) {
		mPostProcessInput = mRenderSys.CreateRenderTexture(mWidth, mHeight, kFormatR16G16B16A16UNorm);
		//SET_DEBUG_NAME(mPostProcessInput->mDepthStencilView, "post_process_input");
	}
	return mPostProcessInput;
}

}