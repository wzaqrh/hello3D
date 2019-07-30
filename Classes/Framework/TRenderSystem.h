#pragma once
#include "TBaseTypes.h"

class TD3DInput;
__declspec(align(16)) class TRenderSystem
{
public:
	HINSTANCE mHInst = NULL;
	HWND mHWnd = NULL;
	D3D_DRIVER_TYPE mDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* mDevice = NULL;
	ID3D11DeviceContext* mDeviceContext = NULL;
	IDXGISwapChain* mSwapChain = NULL;
	ID3D11RenderTargetView* mRenderTargetView = NULL;
	ID3D11Texture2D* mDepthStencil = NULL;
	ID3D11DepthStencilView* mDepthStencilView = NULL;
	ID3D11DepthStencilState* mDepthStencilState = NULL;
	ID3D11BlendState* mBlendState = NULL;
	int mScreenWidth;
	int mScreenHeight;
	TD3DInput* mInput = nullptr;
	TMaterialFactoryPtr mMaterialFac;
	std::map<std::string, ID3D11ShaderResourceView*> mTexByPath;
	XMMATRIX mWorldTransform;
public:
	TCameraPtr mDefCamera;
	std::vector<TPointLightPtr> mPointLights;
	std::vector<TDirectLightPtr> mDirectLights;
	std::vector<TSpotLightPtr> mSpotLights;
public:
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
	TRenderSystem();
	~TRenderSystem();

	HRESULT Initialize();
	void CleanUp();
public:
	void ApplyMaterial(TMaterialPtr material, const XMMATRIX& worldTransform, TCameraBase* pCam=nullptr, TProgramPtr program=nullptr);
public:
	TSpotLightPtr AddSpotLight();
	TPointLightPtr AddPointLight();
	TDirectLightPtr AddDirectLight();
	TCameraPtr SetCamera(double fov, int eyeDistance, double far1);
public:
	TRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT);
	void ClearRenderTexture(TRenderTexturePtr rendTarget, XMFLOAT4 color);
	void SetRenderTarget(TRenderTexturePtr rendTarget);

	TMaterialPtr CreateMaterial(const char* vsPath, 
		const char* psPath, 
		D3D11_INPUT_ELEMENT_DESC* descArray, 
		size_t descCount);

	TContantBufferPtr CreateConstBuffer(int bufferSize);
	TIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer);
	void SetIndexBuffer(TIndexBufferPtr indexBuffer);
	void DrawIndexed(TIndexBufferPtr indexBuffer);

	ID3D11Buffer* _CreateVertexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize);
	TVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr);
	void SetVertexBuffer(TVertexBufferPtr vertexBuffer);

	bool UpdateBuffer(THardwareBuffer* buffer, void* data, int dataSize);
	void UpdateConstBuffer(TContantBufferPtr buffer, void* data);

	ID3D11VertexShader* _CreateVS(const char* filename, ID3DBlob*& pVSBlob);
	ID3D11PixelShader* _CreatePS(const char* filename);
	TProgramPtr CreateProgram(const char* vsPath, const char* psPath);

	ID3D11SamplerState* CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	ID3D11InputLayout* CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);

	ID3D11ShaderResourceView* _CreateTexture(const char* pSrcFile);
	TTexture GetTexByPath(const std::string& __imgPath);

	void SetWorldTransform(const XMMATRIX& transform);

	void SetBlendFunc(const TBlendFunc& blendFunc);
	void SetDepthState(const TDepthState& depthState);
public:
	void Draw(IRenderable* renderable);
	void RenderOperation(const TRenderOperation& op);
};
