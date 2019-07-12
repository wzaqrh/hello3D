#include "TBaseTypes.h"

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