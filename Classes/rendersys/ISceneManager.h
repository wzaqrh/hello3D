#pragma once
#include "ISceneManagerPred.h"
#include "IRenderablePred.h"
#include "TBaseTypes.h"
#include "TMaterialPred.h"

__declspec(align(16)) 
struct TCameraBase {
public:
	XMMATRIX mView_;
	XMMATRIX mProjection_;
	XMMATRIX mWorldView;
public:
	void* operator new(size_t i) { return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
	TCameraBase();
public:
	XMFLOAT3 CalNDC(XMFLOAT3 pos);
	XMFLOAT4 CalNDC(XMFLOAT4 pos);

	virtual const XMMATRIX& GetView();
	virtual const XMMATRIX& GetProjection();
};

typedef std::shared_ptr<struct TTransform> TTransformPtr;
__declspec(align(16)) 
struct TCamera : public TCameraBase {
public:
	TTransformPtr mTransform;
	bool mTransformDirty;

	bool mIsPespective = true;
	bool mFlipY = false;
	float mEyeDistance;
	XMFLOAT3 mEye, mAt, mUp;
	int mWidth, mHeight;
	double mFOV, mFar;
public:
	void* operator new(size_t i) { return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }
	TCamera();
	TCamera(const TCamera& other);
public:
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at);
	void SetPerspectiveProj(int width, int height, double fov, double far1);
	void SetOthogonalProj(int width, int height, double far1);
	TTransformPtr GetTransform();

	const XMMATRIX& GetView() override;
	const XMMATRIX& GetProjection() override;
	int GetWidth() { return mWidth; }
	int GetHeight() { return mHeight; }
	void SetFlipY(bool flip);
private:
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up);
public:
	static std::shared_ptr<TCamera> CreatePerspective(int width, int height, double fov = 45.0, int eyeDistance = 10, double far1 = 100);
	static std::shared_ptr<TCamera> CreateOthogonal(int width, int height, double far1 = 100);
};

MIDL_INTERFACE("CCCACB79-2DB3-4771-9AE5-1CB5369F206C")
ISceneManager : public IUnknown
{
	virtual STDMETHODIMP_(TCameraPtr) GetDefCamera() = 0;

	virtual STDMETHODIMP_(TSpotLightPtr) AddSpotLight() = 0;
	virtual STDMETHODIMP_(TPointLightPtr) AddPointLight() = 0;
	virtual STDMETHODIMP_(TDirectLightPtr) AddDirectLight() = 0;
	virtual STDMETHODIMP_(TCameraPtr) SetOthogonalCamera(double far1) = 0;
	virtual STDMETHODIMP_(TCameraPtr) SetPerspectiveCamera(double fov, int eyeDistance, double far1) = 0;
	virtual STDMETHODIMP_(TSkyBoxPtr) SetSkyBox(const std::string& imgName) = 0;
	virtual STDMETHODIMP_(TPostProcessPtr) AddPostProcess(const std::string& name) = 0;
};