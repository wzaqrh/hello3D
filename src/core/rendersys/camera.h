#pragma once
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/base/transform.h"

namespace mir {

struct __declspec(align(16)) CameraBase
{
public:
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mWorldView;
public:
	CameraBase();
public:
	XMFLOAT3 ProjectPoint(XMFLOAT3 worldpos);//world -> ndc
	XMFLOAT4 ProjectPoint(XMFLOAT4 worldpos);

	virtual const XMMATRIX& GetView();
	virtual const XMMATRIX& GetProjection();
};

struct __declspec(align(16)) Camera : public CameraBase
{
public:
	TransformPtr mTransform;
	bool mTransformDirty;

	bool mIsPespective = true;
	bool mFlipY = false;
	float mEyeDistance;
	XMFLOAT3 mEye, mAt, mUp;
	int mWidth, mHeight;
	double mFOV, mFar;
public:
	Camera();
	Camera(const Camera& other);
public:
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at);
	void SetPerspectiveProj(int width, int height, double fov, double far1);
	void SetOthogonalProj(int width, int height, double far1);
	TransformPtr GetTransform();

	const XMMATRIX& GetView() override;
	const XMMATRIX& GetProjection() override;
	int GetWidth() { return mWidth; }
	int GetHeight() { return mHeight; }
	void SetFlipY(bool flip);
private:
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up);
public:
	static std::shared_ptr<Camera> CreatePerspective(int width, int height, double fov = 45.0, int eyeDistance = 10, double far1 = 100);
	static std::shared_ptr<Camera> CreateOthogonal(int width, int height, double far1 = 100);
};

}