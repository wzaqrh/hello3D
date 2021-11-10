#include "core/rendersys/camera.h"
#include "core/base/utility.h"

namespace mir {

Camera::Camera()
{
	mFlipY = false;
	mTransformDirty = true;
	mIsPespective = true;

	mView = XMMatrixIdentity();
	mProjection = XMMatrixIdentity();

	mTransform = std::make_shared<Transform>();
	mUpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

CameraPtr Camera::CreatePerspective(int width, int height, XMFLOAT3 eyePos, double far1, double fov)
{
	CameraPtr pCam = std::make_shared<Camera>();
	pCam->SetLookAt(eyePos, XMFLOAT3(0, 0, 0));
	pCam->SetPerspectiveProj(width, height, fov, far1);
	return pCam;
}

CameraPtr Camera::CreateOthogonal(int width, int height, XMFLOAT3 eyePos, double far1)
{
	CameraPtr pCam = std::make_shared<Camera>();
	pCam->SetLookAt(eyePos, XMFLOAT3(0, 0, 0));
	pCam->SetOthogonalProj(width, height, far1);
	return pCam;
}

XMFLOAT3 Camera::ProjectPoint(XMFLOAT3 pos)
{
	XMFLOAT3 ret = XMFLOAT3(0, 0, 0);
	XMMATRIX vp  = mView * mProjection;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x, pos.y, pos.z, 1), vp);
	auto w = XMVectorGetW(vec);
	if (w != 0) 
		ret = XMFLOAT3(XMVectorGetX(vec) / w, XMVectorGetY(vec) / w, XMVectorGetZ(vec) / w);
	return ret;
}
XMFLOAT4 Camera::ProjectPoint(XMFLOAT4 pos)
{
	XMMATRIX vp  = mView * mProjection;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x, pos.y, pos.z, pos.z), vp);
	XMFLOAT4 ret = XMFLOAT4(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec), XMVectorGetW(vec));
	return ret;
}

void Camera::SetLookAt(XMFLOAT3 eye, XMFLOAT3 at)
{
	mEyePos = eye;
	mLookAtPos = at;
	XMVECTOR Eye = XMVectorSet(eye.x, eye.y, eye.z, 0.0f);
	XMVECTOR At = XMVectorSet(at.x, at.y, at.z, 0.0f);
	XMVECTOR Up = XMVectorSet(mUpVector.x, mUpVector.y, mUpVector.z, 0.0f);
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

	mTransform->SetPosition(XMFLOAT3(mWidth / 2, mHeight / 2, 0));
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

	mTransform->SetPosition(XMFLOAT3(mWidth / 2, mHeight / 2, 0));
	mTransformDirty = true;
}

TransformPtr Camera::GetTransform()
{
	mTransformDirty = true;
	return mTransform;
}

const XMMATRIX& Camera::GetView()
{
	if (mTransformDirty) {
		mTransformDirty = false;

		auto position = mTransform->GetPosition();
		{
			auto newpos = position;
			auto scale = mTransform->GetScale();
			newpos.x = position.x - scale.x * mWidth / 2;
			newpos.y = position.y - scale.y * mHeight / 2;
			newpos.z = position.z;
			mTransform->SetPosition(newpos);

			auto srt = mTransform->GetMatrixSRT();
			auto worldInv = XM::Inverse(srt);
			auto view = mView;
			mWorldView = worldInv * view;
		}
		mTransform->SetPosition(position);
	}
	return mWorldView;
}

}