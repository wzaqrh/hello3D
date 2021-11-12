#include "core/rendersys/camera.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/render_pipeline.h"
#include "core/base/utility.h"

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

	mView = XMMatrixIdentity();
	mProjection = XMMatrixIdentity();

	mTransform = std::make_shared<Transform>();
	mUpVector = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
}

Eigen::Vector3f Camera::ProjectPoint(const Eigen::Vector3f& pos) const
{
	XMMATRIX vp  = mView * mProjection;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x(), pos.y(), pos.z(), 1), vp);
	auto w = XMVectorGetW(vec);

	Eigen::Vector3f ret = (w != 0) 
		? Eigen::Vector3f(XMVectorGetX(vec) / w, XMVectorGetY(vec) / w, XMVectorGetZ(vec) / w)
		: Eigen::Vector3f(0, 0, 0);
	return ret;
}
Eigen::Vector4f Camera::ProjectPoint(const Eigen::Vector4f& pos) const
{
	XMMATRIX vp  = mView * mProjection;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x(), pos.y(), pos.z(), pos.z()), vp);
	Eigen::Vector4f ret = Eigen::Vector4f(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec), XMVectorGetW(vec));
	return ret;
}

void Camera::SetLookAt(const Eigen::Vector3f& eye, const Eigen::Vector3f& at)
{
	mEyePos = eye;
	mLookAtPos = at;
	XMVECTOR Eye = XMVectorSet(eye.x(), eye.y(), eye.z(), 0.0f);
	XMVECTOR At = XMVectorSet(at.x(), at.y(), at.z(), 0.0f);
	XMVECTOR Up = XMVectorSet(mUpVector.x(), mUpVector.y(), mUpVector.z(), 0.0f);
	mView = XMMatrixLookAtLH(Eye, At, Up);
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
	mWidth = width;
	mHeight = height;
	mZFar = zFar;
	mFov = fov / 180.0 * XM_PI;
	mProjection = XMMatrixPerspectiveFovLH(mFov, mWidth * 1.0 / mHeight, 0.01f, mZFar);
	mIsPespective = true;

	if (mFlipY) mProjection = mProjection * XMMatrixScaling(1, -1, 1);

	mTransform->SetPosition(Eigen::Vector3f(mWidth / 2, mHeight / 2, 0));
	mTransformDirty = true;
}

void Camera::SetOthogonalProj(int width, int height, double zFar)
{
	mWidth = width;
	mHeight = height;
	mZFar = zFar;
	//mProjection_ = XMMatrixOrthographicLH(mWidth, mHeight, 0.01, mFar);
	mProjection = XMMatrixOrthographicOffCenterLH(0, mWidth, 0, mHeight, 0.01, mZFar);
	mIsPespective = false;

	if (mFlipY) mProjection = mProjection * XMMatrixScaling(1, -1, 1);

	mTransform->SetPosition(Eigen::Vector3f(mWidth / 2, mHeight / 2, 0));
	mTransformDirty = true;
}

const TransformPtr& Camera::GetTransform() const
{
	mTransformDirty = true;
	return mTransform;
}

const XMMATRIX& Camera::GetView() const
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

			auto srt = mTransform->SetMatrixSRT();
			auto worldInv = XM::Inverse(srt);
			auto view = mView;
			mWorldView = worldInv * view;
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
		mPostProcessInput = mRenderSys.CreateRenderTexture(mWidth, mHeight, DXGI_FORMAT_R16G16B16A16_UNORM);// , DXGI_FORMAT_R8G8B8A8_UNORM);
		SET_DEBUG_NAME(mPostProcessInput->mDepthStencilView, "post_process_input");
	}
	return mPostProcessInput;
}

}