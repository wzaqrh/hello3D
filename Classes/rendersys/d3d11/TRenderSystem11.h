#pragma once
#include "IRenderSystem.h"
#include "TInterfaceType11.h"

class TRenderSystem11 
	: public TRenderSystem
{
private:
	HWND mHWnd = NULL;
	D3D_DRIVER_TYPE mDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* mDevice = NULL;
	ID3D11DeviceContext* mDeviceContext = NULL;
	IDXGISwapChain* mSwapChain = NULL;
	ID3D11Texture2D* mDepthStencil = NULL;
	ID3D11DepthStencilState* mDepthStencilState = NULL;
	ID3D11BlendState* mBlendState = NULL;

	ID3D11RenderTargetView* mBackRenderTargetView = NULL;
	ID3D11DepthStencilView* mBackDepthStencilView = NULL;
	ID3D11RenderTargetView* mCurRenderTargetView = NULL;
	ID3D11DepthStencilView* mCurDepthStencilView = NULL;

	TThreadPumpPtr mThreadPump;
	std::vector<D3D_SHADER_MACRO> mShaderMacros;
public:
	void* operator new(size_t i){ return _mm_malloc(i,16); }
	void operator delete(void* p) { _mm_free(p); }
	TRenderSystem11();
	~TRenderSystem11();
public:
	STDMETHODIMP_(bool) Initialize(HWND hWnd);
	STDMETHODIMP_(void) Update(float dt);
	STDMETHODIMP_(void) CleanUp();
public:
	STDMETHODIMP_(void) ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil);

	STDMETHODIMP_(IRenderTexturePtr) CreateRenderTexture(int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	void _ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth=1.0, UINT8 Stencil=0);
	STDMETHODIMP_(void) SetRenderTarget(IRenderTexturePtr rendTarget);

	STDMETHODIMP_(TMaterialPtr) CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback);

	STDMETHODIMP_(IContantBufferPtr) CloneConstBuffer(IContantBufferPtr buffer);
	STDMETHODIMP_(IContantBufferPtr) CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data = nullptr);
	STDMETHODIMP_(IIndexBufferPtr) CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer);
	STDMETHODIMP_(void) SetIndexBuffer(IIndexBufferPtr indexBuffer);

	STDMETHODIMP_(IVertexBufferPtr) CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer = nullptr);
	STDMETHODIMP_(void) SetVertexBuffer(IVertexBufferPtr vertexBuffer);

	STDMETHODIMP_(bool) UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize);
	STDMETHODIMP_(void) UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize);

	STDMETHODIMP_(IProgramPtr) CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr);
	STDMETHODIMP_(IProgramPtr) CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);

	STDMETHODIMP_(ISamplerStatePtr) CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER);
	STDMETHODIMP_(IInputLayoutPtr) CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);

	STDMETHODIMP_(void) SetBlendFunc(const TBlendFunc& blendFunc);
	STDMETHODIMP_(void) SetDepthState(const TDepthState& depthState);
public:
	STDMETHODIMP_(bool) BeginScene();
	STDMETHODIMP_(void) EndScene();
	STDMETHODIMP_(void) RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode);
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube);
private:
	HRESULT _CreateDeviceAndSwapChain(int width, int height);
	HRESULT _CreateBackRenderTargetView();
	HRESULT _CreateBackDepthStencilView(int width, int height);
	void _SetViewports(int width, int height);
	HRESULT _SetRasterizerState(); 
protected:
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize);

	TVertexShader11Ptr _CreateVS(const char* filename, const char* entry = nullptr, bool async = true);
	TVertexShader11Ptr _CreateVSByFXC(const char* filename);
	TPixelShader11Ptr _CreatePS(const char* filename, const char* entry = nullptr, bool async = true);
	TPixelShader11Ptr _CreatePSByFXC(const char* filename);

	ID3D11InputLayout* _CreateInputLayout(TProgram11* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr);
protected:
	void BindPass(TPassPtr pass, const cbGlobalParam& globalParam);
	void RenderPass(TPassPtr pass, TTextureBySlot& texturs, int iterCnt, IIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam);
	void RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode);
	void _RenderSkyBox();
	void _DoPostProcess();
};
