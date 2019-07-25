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

__declspec(align(16)) struct TCameraBase {
	XMMATRIX mView;
	XMMATRIX mProjection;
public:
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
public:
	XMFLOAT3 CalNDC(XMFLOAT3 pos);
};

struct TCamera : public TCameraBase {
public:
	XMFLOAT3 mEye, mAt, mUp;
	int mWidth, mHeight;
	double mFOV, mFar;
public:
	TCamera() {}
	TCamera(int width, int height, double fov = 45.0, int eyeDistance = 10, double far1=100);
	TCamera(const TCamera& other);
	void SetProjection(int width, int height, double fov, double far1);
	void SetLookAt(XMFLOAT3 eye, XMFLOAT3 at);
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
public:
	TCameraBase GetLightCamera(TCamera& otherCam);
};
typedef std::shared_ptr<TDirectLight> TDirectLightPtr;

struct TPointLight : public TDirectLight {
	XMFLOAT4 mAttenuation;
public:
	TPointLight();
	void SetPosition(float x, float y, float z);
	void SetAttenuation(float a, float b, float c);
	TCameraBase GetLightCamera(TCamera& otherCam);
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
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	DXGI_FORMAT mFormat;
public:
	TRenderTexture(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT);
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);
	
	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
};
typedef std::shared_ptr<TRenderTexture> TRenderTexturePtr;

enum enTextureType {
	E_TEXTURE_DIFFUSE,
	E_TEXTURE_SPECULAR,
	E_TEXTURE_NORMAL
};
enum enTexturePbrType {
	E_TEXTURE_PBR_ALBEDO,
	E_TEXTURE_PBR_NORMAL,
	E_TEXTURE_PBR_METALNESS,
	E_TEXTURE_PBR_ROUGHNESS,
	E_TEXTURE_PBR_AO
};
struct TTexture {
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	std::string path;
	ID3D11ShaderResourceView *texture;
public:
	TTexture();
	TTexture(std::string __path, ID3D11ShaderResourceView* __texture);
};

struct TTextureBySlot {
	std::vector<TTexture> textures;
public:
	void clear();
	void push_back(const TTexture& texture);
	bool empty() const;
	size_t size() const;
	void swap(TTextureBySlot& other);
	void resize(size_t size);
	const TTexture& At(size_t pos) const;
	TTexture& At(size_t pos);
	const TTexture& operator[](size_t pos) const;
	TTexture& operator[](size_t pos);
	std::vector<ID3D11ShaderResourceView*> GetTextureViews() const;
};

struct TProgram {
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3DBlob* mVSBlob = nullptr;
};
typedef std::shared_ptr<TProgram> TProgramPtr;

struct THardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	THardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	THardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct TVertexBuffer : public THardwareBuffer {
	unsigned int stride, offset;
public:
	TVertexBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:THardwareBuffer(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	TVertexBuffer() :stride(0), offset(0) {};
};
typedef std::shared_ptr<TVertexBuffer> TVertexBufferPtr;

struct TIndexBuffer : public THardwareBuffer {
	DXGI_FORMAT format;
public:
	TIndexBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:THardwareBuffer(__buffer, __bufferSize), format(__format) {};
	TIndexBuffer() :format(DXGI_FORMAT_UNKNOWN) {};
	int GetWidth();
};
typedef std::shared_ptr<TIndexBuffer> TIndexBufferPtr;

struct TContantBuffer : public THardwareBuffer {
	TContantBuffer() {}
	TContantBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :THardwareBuffer(__buffer, __bufferSize) {}
};
typedef std::shared_ptr<TContantBuffer> TContantBufferPtr;

struct TMaterial {
	TProgramPtr mProgram;
	ID3D11InputLayout* mInputLayout = nullptr;

	D3D11_PRIMITIVE_TOPOLOGY mTopoLogy;
	ID3D11SamplerState*	mSampler = nullptr;

	std::vector<ID3D11Buffer*> mConstBuffers;
	std::vector<TContantBufferPtr> mConstantBuffers;
public:
	TContantBufferPtr AddConstBuffer(TContantBufferPtr buffer);
};
typedef std::shared_ptr<TMaterial> TMaterialPtr;

struct TRenderOperation {
	TMaterialPtr mMaterial;
	TVertexBufferPtr mVertexBuffer;
	TIndexBufferPtr mIndexBuffer;
	TTextureBySlot mTextures;
};
typedef std::vector<TRenderOperation> TRenderOperationList;

struct IRenderable {
	virtual int GenRenderOperation(TRenderOperationList& opList) = 0;
};