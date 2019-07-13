#include "TBaseTypes.h"

/********** TCamera **********/
TCamera::TCamera(int width, int height)
{
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mView = XMMatrixLookAtLH(Eye, At, Up);

	/*XMVECTOR res = XMVector4Transform(Eye, mView);
	float x = XMVectorGetX(res);
	float y = XMVectorGetY(res);
	float z = XMVectorGetZ(res);*/

	// Initialize the projection matrix
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);
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
	mPosition = XMFLOAT4(x, y, z, 0);
}

void TDirectLight::SetDiffuseColor(float r, float g, float b, float a)
{
	mDiffuseColor = XMFLOAT4(r, g, b, a);
}

void TDirectLight::SetSpecularColor(float r, float g, float b, float a)
{
	mSpecularColorPower = XMFLOAT4(r, g, b, mSpecularColorPower.w);
}

void TDirectLight::SetSpecularPower(float power)
{
	mSpecularColorPower.w = power;
}

/********** TLight **********/
TPointLight::TPointLight()
{
	SetPosition(0, 0, 0);
	SetAttenuation(1.0, 0.01, 0.0);
}

void TPointLight::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT4(x, y, z, 1);
}

void TPointLight::SetAttenuation(float a, float b, float c)
{
	mAttenuation = XMFLOAT4(a, b, c, 0);
}

/********** cbGlobalParam **********/
cbGlobalParam::cbGlobalParam()
{
	auto Ident = XMMatrixIdentity();
	mWorld = Ident;
	mView = Ident;
	mProjection = Ident;
}