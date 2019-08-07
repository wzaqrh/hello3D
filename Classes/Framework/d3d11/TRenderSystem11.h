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

	TMaterialFactoryPtr mMaterialFac;
	TRenderTexturePtr mShadowPassRT, mPostProcessRT;

	ID3D11RenderTargetView* mBackRenderTargetView = NULL;
	ID3D11DepthStencilView* mBackDepthStencilView = NULL;
	ID3D11RenderTargetView* mCurRenderTargetView = NULL;
	ID3D11DepthStencilView* mCurDepthStencilView = NULL;
	std::vector<TRenderTexturePtr> mRenderTargetStk;

	TThreadPumpPtr mThreadPump;

	bool mCastShdowFlag = false;
	TBlendFunc mCurBlendFunc;
	TDepthState mCurDepthState;

	std::vector<TPostProcessPtr> mPostProcs;
	std::vector<TPointLightPtr> mPointLights;
	std::vector<TDirectLightPtr> mDirectLights;
	std::vector<TSpotLightPtr> mSpotLights;
	std::vector<std::pair<TDirectLight*, enLightType>> mLightsOrder;
public:
	TRenderSystem11();
	~TRenderSystem11();
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
public:
	virtual bool Initialize();
	virtual void Update(float dt);
	virtual void CleanUp();

	virtual TSpotLightPtr AddSpotLight();
	virtual TPointLightPtr AddPointLight();
	virtual TDirectLightPtr AddDirectLight();
	virtual TCameraPtr SetCamera(double fov, int eyeDistance, double far1);
	virtual TSkyBoxPtr SetSkyBox(const std::string& imgName);
	virtual TPostProcessPtr AddPostProcess(const std::string& name);
public:
	virtual void SetHandle(HINSTANCE hInstance, HWND hWnd);
	virtual void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil);

	virtual TRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	virtual void ClearRenderTexture(TRenderTexturePtr rendTarget, XMFLOAT4 color);
	virtual void SetRenderTarget(TRenderTexturePtr rendTarget);

	virtual TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback);

	virtual TContantBufferPtr CloneConstBuffer(TContantBufferPtr buffer);
	virtual TContantBufferPtr CreateConstBuffer(int bufferSize, void* data = nullptr);
	virtual TIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer);
	virtual void SetIndexBuffer(TIndexBufferPtr indexBuffer);
	virtual void DrawIndexed(TIndexBufferPtr indexBuffer);

	virtual IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer = nullptr);
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer);

	virtual bool UpdateBuffer(IHardwareBuffer* buffer, void* data, int dataSize);
	virtual void UpdateConstBuffer(TContantBufferPtr buffer, void* data);

	virtual TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr);
	virtual TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);
	virtual TProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);

	virtual ID3D11SamplerState* CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER);
	virtual TInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);

	virtual void SetBlendFunc(const TBlendFunc& blendFunc);
	virtual void SetDepthState(const TDepthState& depthState);
public:
	virtual bool BeginScene();
	virtual void EndScene();
	virtual void Draw(IRenderable* renderable);
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
	void _PushRenderTarget(TRenderTexturePtr rendTarget);
	void _PopRenderTarget();

	ID3D11Buffer* _CreateVertexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize);

	TVertexShaderPtr _CreateVS(const char* filename, const char* entry = nullptr, bool async = true);
	TVertexShaderPtr _CreateVSByFXC(const char* filename);
	TPixelShaderPtr _CreatePS(const char* filename, const char* entry = nullptr, bool async = true);
	TPixelShaderPtr _CreatePSByFXC(const char* filename);

	ID3D11InputLayout* _CreateInputLayout(TProgram* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr);
protected:
	void RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode);
	void RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderPass(TPassPtr pass, TTextureBySlot& texturs, int iterCnt, TIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam);
	void RenderSkyBox();
	void DoPostProcess();

	cbGlobalParam MakeAutoParam(TCameraBase* pLightCam, bool castShadow, TDirectLight* light, enLightType lightType);
	void BindPass(TPassPtr pass, const cbGlobalParam& globalParam);
};

