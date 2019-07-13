#include "TBaseTypes.h"

/********** TCamera **********/
TCamera::TCamera(int width, int height)
{
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mView = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);
}

/********** TLight **********/
TLight::TLight()
{
	SetPosition(0, 0, 0);
	SetDiffuseColor(1,1,1,1);
	SetSpecularColor(1, 1, 1, 1);
	mSpecularPower = 32;
}

void TLight::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT4(x, y, z, 1);
}

void TLight::SetDiffuseColor(float r, float g, float b, float a)
{
	mDiffuseColor = XMFLOAT4(r, g, b, a);
}

void TLight::SetSpecularColor(float r, float g, float b, float a)
{
	mSpecularColor = XMFLOAT4(r, g, b, a);
}

void TLight::SetSpecularPower(float power)
{
	mSpecularPower = power;
}

/********** cbGlobalParam **********/
cbGlobalParam::cbGlobalParam()
{
	auto Ident = XMMatrixIdentity();
	mWorld = Ident;
	mView = Ident;
	mProjection = Ident;
}
