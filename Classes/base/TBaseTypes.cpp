#include "TBaseTypes.h"
#include "TMovable.h"
#include "TMaterialCB.h"
#include "Utility.h"

TCameraBase::TCameraBase()
{
	mView_ = XMMatrixIdentity();
	mProjection_ = XMMatrixIdentity();
}

XMFLOAT3 TCameraBase::CalNDC(XMFLOAT3 pos)
{
	XMFLOAT3 ret = XMFLOAT3(0,0,0);
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
}

TCamera::TCamera(const TCamera& other)
{
	mIsPespective = other.mIsPespective;
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
	mUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR Eye = XMVectorSet(mEye.x, mEye.y, mEye.z, 0.0f);
	XMVECTOR At = XMVectorSet(mAt.x, mAt.y, mAt.z, 0.0f);
	XMVECTOR Up = XMVectorSet(mUp.x, mUp.y, mUp.z, 0.0f);
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
			auto newpos = position;// XMFLOAT3(position.x - mWidth / 2, position.y - mHeight / 2, 0);
			auto scale = mTransform->GetScale();
			newpos.x *= 1 - scale.x;
			newpos.y *= 1 - scale.y;
			newpos.z *= 1 - scale.z;
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

/********** TDirectLight **********/
TDirectLight::TDirectLight()
{
	SetDirection(0, 0, 1);
	SetDiffuseColor(1, 1, 1, 1);
	SetSpecularColor(1, 1, 1, 1);
	SetSpecularPower(32);
}

void TDirectLight::SetDirection(float x, float y, float z)
{
	LightPos = XMFLOAT4(x, y, z, 0);
}

void TDirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	DiffuseColor = XMFLOAT4(r, g, b, a);
}

void TDirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	SpecularColorPower = XMFLOAT4(r, g, b, SpecularColorPower.w);
}

void TDirectLight::SetSpecularPower(float power)
{
	SpecularColorPower.w = power;
}

TCameraBase TDirectLight::GetLightCamera(TCamera& otherCam)
{
	TCamera ret(otherCam);
	ret.SetLookAt(XMFLOAT3(LightPos.x, LightPos.y, LightPos.z), ret.mAt);
	return ret;
}

TConstBufferDecl& TDirectLight::GetDesc()
{
	static TConstBufferDecl decl;
	TConstBufferDeclBuilder builder(decl);
	TDirectLight cb;
	BUILD_ADD(LightPos);
	BUILD_ADD(DiffuseColor);
	BUILD_ADD(SpecularColorPower);
	return builder.Build();
}

/********** TLight **********/
TPointLight::TPointLight()
{
	SetPosition(0, 0, -10);
	SetAttenuation(1.0, 0.01, 0.0);
}

void TPointLight::SetPosition(float x, float y, float z)
{
	LightPos = XMFLOAT4(x, y, z, 1);
}

void TPointLight::SetAttenuation(float a, float b, float c)
{
	Attenuation = XMFLOAT4(a, b, c, 0);
}

TConstBufferDecl& TPointLight::GetDesc()
{
	static TConstBufferDecl decl;
	decl = TDirectLight::GetDesc();

	TConstBufferDeclBuilder builder(decl);
	TPointLight cb;
	BUILD_ADD(Attenuation);
	return builder.Build();
}

/********** TSpotLight **********/
TSpotLight::TSpotLight()
{
	SetDirection(0, 0, 1);
	SetAngle(3.14 * 30 / 180);
}

void TSpotLight::SetDirection(float x, float y, float z)
{
	DirectionCutOff = XMFLOAT4(x, y, z, DirectionCutOff.w);
}

void TSpotLight::SetCutOff(float cutoff)
{
	DirectionCutOff.w = cutoff;
}

void TSpotLight::SetAngle(float radian)
{
	SetCutOff(cos(radian));
}

TConstBufferDecl& TSpotLight::GetDesc()
{
	static TConstBufferDecl decl;
	decl = TPointLight::GetDesc();

	TConstBufferDeclBuilder builder(decl);
	TSpotLight cb;
	BUILD_ADD(DirectionCutOff);
	return builder.Build();
}

/********** TBlendFunc **********/
TBlendFunc::TBlendFunc(D3D11_BLEND __src, D3D11_BLEND __dst)
{
	src = __src;
	dst = __dst;
}

TBlendFunc TBlendFunc::DISABLE = TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
TBlendFunc TBlendFunc::ALPHA_PREMULTIPLIED = TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA);
TBlendFunc TBlendFunc::ALPHA_NON_PREMULTIPLIED = TBlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
TBlendFunc TBlendFunc::ADDITIVE = TBlendFunc(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE);

/********** TDepthState **********/
TDepthState::TDepthState(bool __depthEnable, D3D11_COMPARISON_FUNC __depthFunc /*= D3D11_COMPARISON_LESS*/, D3D11_DEPTH_WRITE_MASK __depthWriteMask /*= D3D11_DEPTH_WRITE_MASK_ALL*/)
{
	depthEnable = __depthEnable;
	depthFunc = __depthFunc;
	depthWriteMask = __depthWriteMask;
}

TDepthState TDepthState::For2D = TDepthState(false, D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK_ZERO);
TDepthState TDepthState::For3D = TDepthState(true, D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK_ALL);

/********** TData **********/
TData::TData(void* __data, unsigned int __dataSize)
	:data(__data)
	, dataSize(__dataSize)
{
}
