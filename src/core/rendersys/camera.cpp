#include "core/rendersys/camera.h"
#include "core/base/utility.h"

namespace mir {

/********** TCameraBase **********/
CameraBase::CameraBase()
{
	mView_ = XMMatrixIdentity();
	mProjection_ = XMMatrixIdentity();
}

XMFLOAT3 CameraBase::CalNDC(XMFLOAT3 pos)
{
	XMFLOAT3 ret = XMFLOAT3(0, 0, 0);
	XMMATRIX vp = mView_ * mProjection_;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x, pos.y, pos.z, 1), vp);
	auto w = XMVectorGetW(vec);
	if (w != 0) {
		ret = XMFLOAT3(XMVectorGetX(vec) / w, XMVectorGetY(vec) / w, XMVectorGetZ(vec) / w);
	}
	return ret;
}

XMFLOAT4 CameraBase::CalNDC(XMFLOAT4 pos)
{
	XMMATRIX vp = mView_ * mProjection_;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x, pos.y, pos.z, pos.z), vp);
	XMFLOAT4 ret = XMFLOAT4(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec), XMVectorGetW(vec));
	return ret;
}

const XMMATRIX& CameraBase::GetView()
{
	return mView_;
}

//void TCameraBase::SetView(const XMMATRIX& view)
//{
//	mView_ = view;
//}

const XMMATRIX& CameraBase::GetProjection()
{
	return mProjection_;
}

//void TCameraBase::SetProjection(const XMMATRIX& projection)
//{
//	mProjection_ = projection;
//}

/********** TCamera **********/
Camera::Camera()
{
	mTransform = std::make_shared<Transform>();
	mUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

Camera::Camera(const Camera& other)
{
	mIsPespective = other.mIsPespective;
	mFlipY = other.mFlipY;
	mEye = other.mEye;
	mAt = other.mAt;
	mUp = other.mUp;
	mWidth = other.mWidth;
	mHeight = other.mHeight;
	mFOV = other.mFOV;
	mFar = other.mFar;

	mView_ = other.mView_;
	mProjection_ = other.mProjection_;

	mTransformDirty = true;
	mTransform = std::make_shared<Transform>(*other.mTransform);
}

CameraPtr Camera::CreatePerspective(int width, int height, double fov /*= 45.0*/, int eyeDistance /*= 10*/, double far1 /*= 100*/)
{
	CameraPtr pCam = std::make_shared<Camera>();
	pCam->mEyeDistance = eyeDistance;
	pCam->SetLookAt(XMFLOAT3(0.0f, 0.0f, -eyeDistance), XMFLOAT3(0, 0, 0));
	pCam->SetPerspectiveProj(width, height, fov, far1);
	return pCam;
}

CameraPtr Camera::CreateOthogonal(int width, int height, double far1 /*= 100*/)
{
	CameraPtr pCam = std::make_shared<Camera>();
	pCam->SetLookAt(XMFLOAT3(0.0f, 0.0f, -10), XMFLOAT3(0, 0, 0));
	pCam->SetOthogonalProj(width, height, far1);
	return pCam;
}

void Camera::SetLookAt(XMFLOAT3 eye, XMFLOAT3 at)
{
	mEye = eye;
	mAt = at;
	SetLookAt(mEye, mAt, mUp);
}

void Camera::SetFlipY(bool flip)
{
	mFlipY = flip;
	if (mIsPespective) SetPerspectiveProj(mWidth, mHeight, mFOV, mFar);
	else SetOthogonalProj(mWidth, mHeight, mFar);
}

void Camera::SetLookAt(XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up)
{
	XMVECTOR Eye = XMVectorSet(eye.x, eye.y, eye.z, 0.0f);
	XMVECTOR At = XMVectorSet(at.x, at.y, at.z, 0.0f);
	XMVECTOR Up = XMVectorSet(up.x, up.y, up.z, 0.0f);
	mView_ = XMMatrixLookAtLH(Eye, At, Up);
	mTransformDirty = true;
}

void Camera::SetPerspectiveProj(int width, int height, double fov, double far1)
{
	mWidth = width;
	mHeight = height;
	mFar = far1;
	mFOV = fov / 180.0 * XM_PI;
	mProjection_ = XMMatrixPerspectiveFovLH(mFOV, mWidth * 1.0 / mHeight, 0.01f, mFar);
	mIsPespective = true;

	if (mFlipY) mProjection_ = mProjection_ * XMMatrixScaling(1, -1, 1);

	mTransform->SetPosition(XMFLOAT3(mWidth / 2, mHeight / 2, 0));
	mTransformDirty = true;
}

void Camera::SetOthogonalProj(int width, int height, double far1)
{
	mWidth = width;
	mHeight = height;
	mFar = far1;
	//mProjection_ = XMMatrixOrthographicLH(mWidth, mHeight, 0.01, mFar);
	mProjection_ = XMMatrixOrthographicOffCenterLH(0, mWidth, 0, mHeight, 0.01, mFar);
	mIsPespective = false;

	if (mFlipY) mProjection_ = mProjection_ * XMMatrixScaling(1, -1, 1);

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
			auto view = mView_;
			mWorldView = worldInv * view;
		}
		mTransform->SetPosition(position);
	}
	return mWorldView;
}

const XMMATRIX& Camera::GetProjection()
{
	return mProjection_;
}

}