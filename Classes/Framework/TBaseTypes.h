#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <dinput.h>
#include <dxerr.h>
#include "std.h"

#define USE_ONLY_PNG
#define USE_RENDER_OP
#define D3D11_DEBUG

template<class T>
T clamp(T minVal, T maxVal, T v) {
	return min(max(v, minVal),maxVal);
}

struct TINT4 {
	int x, y, z, w;
};

__declspec(align(16)) struct TCameraBase {
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
public:
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
	TCameraBase();
public:
	XMFLOAT3 CalNDC(XMFLOAT3 pos);
	XMFLOAT4 CalNDC(XMFLOAT4 pos);
};

struct TCamera : public TCameraBase {
public:
	float mEyeDistance;
	XMFLOAT3 mEye, mAt, mUp;
	int mWidth, mHeight;
	double mFOV, mFar;
public:
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
	TCamera() {}
	TCamera(int width, int height, double fov = 45.0, int eyeDistance = 10, double far1=100);
	TCamera(const TCamera& other);
	void SetProjection(int width, int height, double fov, double far1);
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at);
};
typedef std::shared_ptr<TCamera> TCameraPtr;

enum enLightType {
	E_LIGHT_DIRECT,
	E_LIGHT_POINT,
	E_LIGHT_SPOT
};

class TDirectLight {
public:
	XMFLOAT4 mPosition;//world space
	XMFLOAT4 mDiffuseColor;
	XMFLOAT4 mSpecularColorPower;
public:
	TDirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);
public:
	TCameraBase GetLightCamera(TCamera& otherCam);
};
typedef std::shared_ptr<TDirectLight> TDirectLightPtr;

class TPointLight : public TDirectLight {
public:
	XMFLOAT4 mAttenuation;
public:
	TPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
	TCameraBase GetLightCamera(TCamera& otherCam);
};
typedef std::shared_ptr<TPointLight> TPointLightPtr;

class TSpotLight : public TPointLight {
public:
	XMFLOAT4 mDirCutOff;
public:
	TSpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);
};
typedef std::shared_ptr<TSpotLight> TSpotLightPtr;

#define MAX_LIGHTS 4
__declspec(align(16)) struct cbGlobalParam
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mWorldInv;
	XMMATRIX mViewInv;
	XMMATRIX mProjectionInv;

	XMINT4 mLightNum;//directional,point,spot
	TDirectLight mDirectLights[MAX_LIGHTS];
	TPointLight mPointLights[MAX_LIGHTS];
	TSpotLight mSpotLights[MAX_LIGHTS];

	XMMATRIX mLightView;
	XMMATRIX mLightProjection;
	BOOL HasDepthMap;
public:
	cbGlobalParam();
};

struct TBlendFunc {
	D3D11_BLEND src,dst;
	TBlendFunc() :src(D3D11_BLEND_ONE), dst(D3D11_BLEND_INV_SRC_ALPHA) {};
	TBlendFunc(D3D11_BLEND __src, D3D11_BLEND __dst);
};

struct TDepthState {
	BOOL depthEnable;
	D3D11_COMPARISON_FUNC depthFunc;
	D3D11_DEPTH_WRITE_MASK depthWriteMask;
	TDepthState() :depthEnable(false), depthFunc(D3D11_COMPARISON_LESS), depthWriteMask(D3D11_DEPTH_WRITE_MASK_ALL) {}
	TDepthState(bool __depthEnable, D3D11_COMPARISON_FUNC __depthFunc = D3D11_COMPARISON_LESS, D3D11_DEPTH_WRITE_MASK __depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL);
};

#include "TPredefine.h"
//#include "TInterfaceType.h"
//#include "TMaterial.h"