#pragma once
#include "core/rendersys/render_system.h"
#include "interface_type11.h"

namespace mir {

class TRenderSystem11 : public TRenderSystem
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
	bool Initialize(HWND hWnd, RECT vp);
	void Update(float dt);
	void CleanUp();
	void SetViewPort(int x, int y, int w, int h);
public:
	void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil);

	IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
	void _ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth=1.0, UINT8 Stencil=0);
	void SetRenderTarget(IRenderTexturePtr rendTarget);

	TMaterialPtr GetMaterial(const std::string& name, bool sharedUse);

	IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer);
	IContantBufferPtr CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data = nullptr);
	IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer);
	void SetIndexBuffer(IIndexBufferPtr indexBuffer);

	IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer = nullptr);
	void SetVertexBuffer(IVertexBufferPtr vertexBuffer);

	bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize);
	void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize);

	IProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr);
	IProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);

	ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER);
	IInputLayoutPtr CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);

	void SetBlendFunc(const TBlendFunc& blendFunc);
	void SetDepthState(const TDepthState& depthState);

	ITexturePtr CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap);
	bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep);
public:
	bool BeginScene();
	void EndScene();
	void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode);
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube);
private:
	HRESULT _CreateDeviceAndSwapChain(int width, int height);
	HRESULT _CreateBackRenderTargetView();
	HRESULT _CreateBackDepthStencilView(int width, int height);
	void _SetViewports(int width, int height, int x = 0, int y = 0);
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
	void BindPass(const TPassPtr& pass, const cbGlobalParam& globalParam);
	void RenderPass(const TPassPtr& pass, TTextureBySlot& texturs, int iterCnt, const TRenderOperation& op, const cbGlobalParam& globalParam);
	void RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode);
	void _RenderSkyBox();
	void _DoPostProcess();
};

}