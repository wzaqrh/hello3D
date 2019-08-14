#pragma once
#include "IRenderSystem.h"

class TRenderSystem11 
	: public IRenderSystem
{
private:
	HINSTANCE mHInst = NULL;
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
	std::vector<IRenderTexturePtr> mRenderTargetStk;

	TThreadPumpPtr mThreadPump;

	bool mCastShdowFlag = false;
	TBlendFunc mCurBlendFunc;
	TDepthState mCurDepthState;
public:
	void* operator new(size_t i){ return _mm_malloc(i,16); }
	void operator delete(void* p) { _mm_free(p); }
	TRenderSystem11();
	~TRenderSystem11();
public:
	virtual bool Initialize();
	virtual void Update(float dt);
	virtual void CleanUp();
public:
	virtual void SetHandle(HINSTANCE hInstance, HWND hWnd);
	virtual void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil);

	virtual IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	void _ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth=1.0, UINT8 Stencil=0);
	virtual void SetRenderTarget(IRenderTexturePtr rendTarget);

	virtual TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback);

	virtual IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer);
	virtual IContantBufferPtr CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data = nullptr);
	virtual IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer);
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer);

	virtual IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer = nullptr);
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer);

	virtual bool UpdateBuffer(IHardwareBuffer* buffer, void* data, int dataSize);
	virtual void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize);

	virtual TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr);
	virtual TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);

	virtual ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER);
	virtual IInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);

	virtual void SetBlendFunc(const TBlendFunc& blendFunc);
	virtual void SetDepthState(const TDepthState& depthState);
public:
	virtual bool BeginScene();
	virtual void EndScene();
	virtual void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode);
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = false);
private:
	HRESULT _CreateDeviceAndSwapChain(int width, int height);
	HRESULT _CreateBackRenderTargetView();
	HRESULT _CreateBackDepthStencilView(int width, int height);
	void _SetViewports(int width, int height);
	HRESULT _SetRasterizerState();
protected:
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();

	ID3D11Buffer* _CreateVertexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize);

	IVertexShaderPtr _CreateVS(const char* filename, const char* entry = nullptr, bool async = true);
	IVertexShaderPtr _CreateVSByFXC(const char* filename);
	IPixelShaderPtr _CreatePS(const char* filename, const char* entry = nullptr, bool async = true);
	IPixelShaderPtr _CreatePSByFXC(const char* filename);

	ID3D11InputLayout* _CreateInputLayout(TProgram* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr);
protected:
	void RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode);
	void RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderPass(TPassPtr pass, TTextureBySlot& texturs, int iterCnt, IIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam);
	void RenderSkyBox();
	void DoPostProcess();

	cbGlobalParam MakeAutoParam(TCameraBase* pLightCam, bool castShadow, TDirectLight* light, enLightType lightType);
	void BindPass(TPassPtr pass, const cbGlobalParam& globalParam);
};

