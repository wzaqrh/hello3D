#pragma once
#include "TBaseTypes.h"

class TRenderSystem
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
	IDirectInput* mInput = NULL;
	IDirectInputDevice* mMouse = NULL;
	int mScreenWidth;
	int mScreenHeight;
public:
	TCameraPtr mDefCamera;
	TLightPtr mDefLight;
public:
	TRenderSystem();
	~TRenderSystem();

	HRESULT Initialize();
	void CleanUp();

	void ApplyMaterial(TMaterialPtr material, const XMMATRIX& worldTransform);
public:
	TMaterialPtr CreateMaterial(const char* vsPath, 
		const char* psPath, 
		D3D11_INPUT_ELEMENT_DESC* descArray, 
		size_t descCount);

	ID3D11Buffer* CreateConstBuffer(int bufferSize);
	ID3D11Buffer* CreateIndexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* CreateVertexBuffer(int bufferSize, void* buffer);
	bool UpdateBuffer(ID3D11Buffer* buffer, void* data, int dataSize);

	ID3D11VertexShader* CreateVS(const char* filename, ID3DBlob*& pVSBlob);
	ID3D11PixelShader* CreatePS(const char* filename);
	ID3D11SamplerState* CreateSampler();
	ID3D11InputLayout* CreateLayout(ID3DBlob* pVSBlob, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);

	ID3D11ShaderResourceView* CreateTexture(const char* pSrcFile);
	ID3D11ShaderResourceView* GetTexByPath(const std::string& __imgPath);
};
