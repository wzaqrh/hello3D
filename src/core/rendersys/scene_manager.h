#pragma once
#include "core/rendersys/scene_manager_pred.h"
#include "core/rendersys/interface_type_pred.h"
#include "core/renderable/renderable_pred.h"
#include "core/rendersys/material_pred.h"
#include "core/rendersys/base_type.h"

namespace mir {

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

interface ISceneManager 
{
	virtual TCameraPtr SetOthogonalCamera(double far1) = 0;
	virtual TCameraPtr SetPerspectiveCamera(double fov, int eyeDistance, double far1) = 0;
	virtual TCameraPtr GetDefCamera() = 0;

	virtual TSpotLightPtr AddSpotLight() = 0;
	virtual TPointLightPtr AddPointLight() = 0;
	virtual TDirectLightPtr AddDirectLight() = 0;

	virtual TSkyBoxPtr GetSkyBox() = 0;
	virtual TSkyBoxPtr SetSkyBox(const std::string& imgName) = 0;
	virtual TPostProcessPtr AddPostProcess(const std::string& name) = 0;
};

struct TSceneManager : public ISceneManager
{
public:
	IRenderSystem* mRenderSys;
	IRenderTexturePtr mPostProcessRT;
	int mScreenWidth, mScreenHeight;
	

	TCameraPtr mDefCamera;
	TSkyBoxPtr mSkyBox;
	std::vector<TPostProcessPtr> mPostProcs;

	std::vector<TDirectLightPtr> mDirectLights;
	std::vector<TPointLightPtr> mPointLights;
	std::vector<TSpotLightPtr> mSpotLights;
	std::vector<std::pair<TDirectLight*, enLightType>> mLightsOrder;
public:
	TSceneManager(IRenderSystem* renderSys, XMINT2 screenSize, IRenderTexturePtr postRT, TCameraPtr defCamera);

	TCameraPtr SetOthogonalCamera(double far1) override;
	TCameraPtr SetPerspectiveCamera(double fov, int eyeDistance, double far1) override;
	TCameraPtr GetDefCamera() {
		return mDefCamera;
	}

	TSpotLightPtr AddSpotLight() override;
	TPointLightPtr AddPointLight() override;
	TDirectLightPtr AddDirectLight() override;

	TSkyBoxPtr SetSkyBox(const std::string& imgName) override;
	TSkyBoxPtr GetSkyBox() {
		return mSkyBox;
	};

	TPostProcessPtr AddPostProcess(const std::string& name) override;
};
};