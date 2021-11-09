#pragma once
#include "core/rendersys/render_system.h"
#include "interface_type11.h"

namespace mir {

class RenderSystem11 : public RenderSystem
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

	ThreadPumpPtr mThreadPump;
	std::vector<D3D_SHADER_MACRO> mShaderMacros;
public:
	void* operator new(size_t i){ return _mm_malloc(i,16); }
	void operator delete(void* p) { _mm_free(p); }
	RenderSystem11();
	~RenderSystem11();
public:
	bool Initialize(HWND hWnd, RECT vp) override;
	void Update(float dt) override;
	void CleanUp() override;
	void SetViewPort(int x, int y, int w, int h) override;
public:
	void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT depth, UINT8 stencil) override;

	IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format) override;
	void _ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth=1.0, UINT8 Stencil=0);
	void SetRenderTarget(IRenderTexturePtr rendTarget) override;

	IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) override;
	IContantBufferPtr CreateConstBuffer(const ConstBufferDecl& cbDecl, void* data) override;
	IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) override;
	void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;

	IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer) override;
	void SetVertexBuffer(IVertexBufferPtr vertexBuffer) override;

	bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) override;
	void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) override;
	void SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program) override;

	IProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath, const char* vsEntry, const char* psEntry) override;
	IProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry, const char* psEntry) override;
	void SetProgram(IProgramPtr program) override;

	ISamplerStatePtr CreateSampler(D3D11_FILTER filter, D3D11_COMPARISON_FUNC comp) override;
	void SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count) override;

	IInputLayoutPtr CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) override;
	void SetVertexLayout(IInputLayoutPtr layout) override;

	void SetBlendFunc(const BlendFunc& blendFunc) override;
	void SetDepthState(const DepthState& depthState) override;

	ITexturePtr CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap) override;
	bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) override;
	void SetTexture(size_t slot, ITexturePtr texture) override;
	void SetTextures(size_t slot, ITexturePtr textures[], size_t count) override;

	void DrawPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) override;
	void DrawIndexedPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) override;

	bool BeginScene() override;
	void EndScene() override;
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube);
private:
	HRESULT _CreateDeviceAndSwapChain(int width, int height);
	HRESULT _CreateBackRenderTargetView();
	HRESULT _CreateBackDepthStencilView(int width, int height);
	void _SetViewports(int width, int height, int x = 0, int y = 0);
	HRESULT _SetRasterizerState(); 
private:
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize);

	VertexShader11Ptr _CreateVS(const char* filename, const char* entry = nullptr, bool async = true);
	VertexShader11Ptr _CreateVSByFXC(const char* filename);
	PixelShader11Ptr _CreatePS(const char* filename, const char* entry = nullptr, bool async = true);
	PixelShader11Ptr _CreatePSByFXC(const char* filename);

	ID3D11InputLayout* _CreateInputLayout(Program11* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr);
};

}