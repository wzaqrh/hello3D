#include "core/rendersys/scene_manager.h"
#include "core/base/transform.h"
#include "core/rendersys/material_cb.h"
#include "core/renderable/skybox.h"
#include "core/renderable/post_process.h"
#include "core/base/utility.h"

namespace mir {

TCameraBase* TDirectLight::GetLightCamera(TCamera& otherCam)
{
	static TCamera ret;
	ret = otherCam;
	ret.SetLookAt(XMFLOAT3(LightPos.x, LightPos.y, LightPos.z), ret.mAt);
	return &ret;
}

/********** TCameraBase **********/
TCameraBase::TCameraBase()
{
	mView_ = XMMatrixIdentity();
	mProjection_ = XMMatrixIdentity();
}

XMFLOAT3 TCameraBase::CalNDC(XMFLOAT3 pos)
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

XMFLOAT4 TCameraBase::CalNDC(XMFLOAT4 pos)
{
	XMMATRIX vp = mView_ * mProjection_;
	XMVECTOR vec = XMVector3Transform(XMVectorSet(pos.x, pos.y, pos.z, pos.z), vp);
	XMFLOAT4 ret = XMFLOAT4(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec), XMVectorGetW(vec));
	return ret;
}

const XMMATRIX& TCameraBase::GetView()
{
	return mView_;
}

//void TCameraBase::SetView(const XMMATRIX& view)
//{
//	mView_ = view;
//}

const XMMATRIX& TCameraBase::GetProjection()
{
	return mProjection_;
}

//void TCameraBase::SetProjection(const XMMATRIX& projection)
//{
//	mProjection_ = projection;
//}

/********** TCamera **********/
TCamera::TCamera()
{
	mTransform = std::make_shared<TTransform>();
	mUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

TCamera::TCamera(const TCamera& other)
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
	mTransform = std::make_shared<TTransform>(*other.mTransform);
}

TCameraPtr TCamera::CreatePerspective(int width, int height, double fov /*= 45.0*/, int eyeDistance /*= 10*/, double far1 /*= 100*/)
{
	TCameraPtr pCam = std::make_shared<TCamera>();
	pCam->mEyeDistance = eyeDistance;
	pCam->SetLookAt(XMFLOAT3(0.0f, 0.0f, -eyeDistance), XMFLOAT3(0, 0, 0));
	pCam->SetPerspectiveProj(width, height, fov, far1);
	return pCam;
}

TCameraPtr TCamera::CreateOthogonal(int width, int height, double far1 /*= 100*/)
{
	TCameraPtr pCam = std::make_shared<TCamera>();
	pCam->SetLookAt(XMFLOAT3(0.0f, 0.0f, -10), XMFLOAT3(0, 0, 0));
	pCam->SetOthogonalProj(width, height, far1);
	return pCam;
}

void TCamera::SetLookAt(XMFLOAT3 eye, XMFLOAT3 at)
{
	mEye = eye;
	mAt = at;
	SetLookAt(mEye, mAt, mUp);
}

void TCamera::SetFlipY(bool flip)
{
	mFlipY = flip;
	if (mIsPespective) {
		SetPerspectiveProj(mWidth, mHeight, mFOV, mFar);
	}
	else {
		SetOthogonalProj(mWidth, mHeight, mFar);
	}
}

void TCamera::SetLookAt(XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up)
{
	XMVECTOR Eye = XMVectorSet(eye.x, eye.y, eye.z, 0.0f);
	XMVECTOR At = XMVectorSet(at.x, at.y, at.z, 0.0f);
	XMVECTOR Up = XMVectorSet(up.x, up.y, up.z, 0.0f);
	mView_ = XMMatrixLookAtLH(Eye, At, Up);
}

void TCamera::SetPerspectiveProj(int width, int height, double fov, double far1)
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

void TCamera::SetOthogonalProj(int width, int height, double far1)
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

TTransformPtr TCamera::GetTransform()
{
	mTransformDirty = true;
	return mTransform;
}

const XMMATRIX& TCamera::GetView()
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

const XMMATRIX& TCamera::GetProjection()
{
	return mProjection_;
}

//TSceneManager
TSceneManager::TSceneManager(IRenderSystem* renderSys, XMINT2 screenSize, IRenderTexturePtr postRT, TCameraPtr defCamera)
{
	mRenderSys = renderSys;
	mScreenWidth  = screenSize.x;
	mScreenHeight = screenSize.y;
	mPostProcessRT = postRT;
	mDefCamera = defCamera;
}

TSpotLightPtr TSceneManager::AddSpotLight()
{
	TSpotLightPtr light = std::make_shared<TSpotLight>();
	mSpotLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_SPOT));
	return light;
}

TPointLightPtr TSceneManager::AddPointLight()
{
	TPointLightPtr light = std::make_shared<TPointLight>();
	mPointLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_POINT));
	return light;
}

TDirectLightPtr TSceneManager::AddDirectLight()
{
	TDirectLightPtr light = std::make_shared<TDirectLight>();
	mDirectLights.push_back(light);
	mLightsOrder.push_back(std::pair<TDirectLight*, enLightType>(light.get(), E_LIGHT_DIRECT));
	return light;
}

TCameraPtr TSceneManager::SetOthogonalCamera(double far1)
{
	mDefCamera = TCamera::CreateOthogonal(mScreenWidth, mScreenHeight, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

TCameraPtr TSceneManager::SetPerspectiveCamera(double fov, int eyeDistance, double far1)
{
	mDefCamera = TCamera::CreatePerspective(mScreenWidth, mScreenHeight, fov, eyeDistance, far1);
	if (mSkyBox) mSkyBox->SetRefCamera(mDefCamera);
	return mDefCamera;
}

TSkyBoxPtr TSceneManager::SetSkyBox(const std::string& imgName)
{
	mSkyBox = std::make_shared<TSkyBox>(mRenderSys, mDefCamera, imgName);
	return mSkyBox;
}

TPostProcessPtr TSceneManager::AddPostProcess(const std::string& name)
{
	TPostProcessPtr process;
	if (name == E_PASS_POSTPROCESS) {
		TBloom* bloom = new TBloom(mRenderSys, mPostProcessRT);
		process = std::shared_ptr<TPostProcess>(bloom);
	}

	if (process) mPostProcs.push_back(process);
	return process;
}

}