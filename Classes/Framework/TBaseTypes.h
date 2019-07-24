#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <dinput.h>
#include <dxerr.h>
#include "std.h"

template<class T>
T clamp(T minVal, T maxVal, T v) {
	return min(max(v, minVal),maxVal);
}

struct TINT4 {
	int x, y, z, w;
};

struct TCamera {
	XMMATRIX mView;
	XMMATRIX mProjection;
public:
	TCamera(int width, int height, double fov = 45.0, int eyeDistance = 10, double far1=100);
};
typedef std::shared_ptr<TCamera> TCameraPtr;

struct TDirectLight {
	XMFLOAT4 mPosition;//world space
	XMFLOAT4 mDiffuseColor;
	XMFLOAT4 mSpecularColorPower;
public:
	TDirectLight();
	void SetDirection(float x, float y, float z);
	void SetDiffuseColor(float r, float g, float b, float a);
	void SetSpecularColor(float r, float g, float b, float a);
	void SetSpecularPower(float power);
};
typedef std::shared_ptr<TDirectLight> TDirectLightPtr;

struct TPointLight : public TDirectLight {
	XMFLOAT4 mAttenuation;
public:
	TPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
};
typedef std::shared_ptr<TPointLight> TPointLightPtr;

struct TSpotLight : public TPointLight {
	XMFLOAT4 mDirCutOff;
public:
	TSpotLight();
	void SetDirection(float x, float y, float z);
	void SetCutOff(float cutoff);
	void SetAngle(float radian);
};
typedef std::shared_ptr<TSpotLight> TSpotLightPtr;

struct TMaterial {
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3D11InputLayout* mInputLayout = nullptr;
	
	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	ID3D11SamplerState*	mSampler = nullptr;

	std::vector<ID3D11Buffer*> mConstantBuffers;
public:
	ID3D11Buffer* AddConstBuffer(ID3D11Buffer* buffer);
};
typedef std::shared_ptr<TMaterial> TMaterialPtr;

#define MAX_LIGHTS 4
struct cbGlobalParam
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMMATRIX mViewInv;

	XMINT4 mLightNum;//directional,point,spot
	TDirectLight mDirectLights[MAX_LIGHTS];
	TPointLight mPointLights[MAX_LIGHTS];
	TSpotLight mSpotLights[MAX_LIGHTS];
public:
	cbGlobalParam();
};

class TRenderTexture {
public:
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11ShaderResourceView* mShaderResourceView;
public:
	TRenderTexture(ID3D11Device* pDevice, int width, int height);
private:
	bool InitTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTargetView(ID3D11Device* pDevice);
	bool InitResourceView(ID3D11Device* pDevice);
};
typedef std::shared_ptr<TRenderTexture> TRenderTexturePtr;