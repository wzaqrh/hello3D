#pragma once
#include <boost/noncopyable.hpp>
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/base/transform.h"

namespace mir {

struct __declspec(align(16)) Camera : boost::noncopyable
{
public:
	static CameraPtr CreatePerspective(int width, int height, 
		XMFLOAT3 eyePos = XMFLOAT3(0,0,-10), 
		double far1 = 100, double fov = 45.0);
	static CameraPtr CreateOthogonal(int width, int height, 
		XMFLOAT3 eyePos = XMFLOAT3(0,0,-10),
		double far1 = 100);
	Camera();
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at);
	void SetPerspectiveProj(int width, int height, double fov, double zFar);
	void SetOthogonalProj(int width, int height, double zFar);
	void SetFlipY(bool flip);
public:
	TransformPtr GetTransform();

	const XMMATRIX& GetView();
	const XMMATRIX& GetProjection() const  { return mProjection; }
	
	int GetWidth() const { return mWidth; }
	int GetHeight() const  { return mHeight; }
	
	XMFLOAT3 ProjectPoint(XMFLOAT3 worldpos);//world -> ndc
	XMFLOAT4 ProjectPoint(XMFLOAT4 worldpos);
private:
	XMMATRIX mView, mProjection, mWorldView;
	bool mFlipY;

	bool mTransformDirty;
	TransformPtr mTransform;
public:
	int mWidth, mHeight;
	XMFLOAT3 mEyePos, mLookAtPos, mUpVector;

	bool mIsPespective;
	double mFov, mZFar;
};

}