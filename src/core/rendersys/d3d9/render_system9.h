#pragma once
#include "core/rendersys/render_system.h"
#include "interface_type9.h"

namespace mir {

class RenderSystem9 : public RenderSystem
{
	HWND mHWnd = NULL;

	D3DCAPS9 mD3DCaps;
	IDirect3D9 *mD3D9 = NULL; // Used to create the D3DDevice
	IDirect3DDevice9 *mDevice9 = NULL; // Our rendering device

	IDirect3DSurface9 *mBackColorBuffer = NULL, *mBackDepthStencilBuffer = NULL;
	IDirect3DSurface9 *mCurColorBuffer = NULL, *mCurDepthStencilBuffer = NULL;

	std::vector<D3DXMACRO> mShaderMacros;
public:
	RenderSystem9();
	virtual ~RenderSystem9();

	bool Initialize(HWND hWnd, RECT vp) override;
	void Update(float dt) override;
	void CleanUp() override;
	void SetViewPort(int x, int y, int w, int h);
public:
	void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil) override;

	IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT) override;
	void SetRenderTarget(IRenderTexturePtr rendTarget) override;

	IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) override;
	IContantBufferPtr CreateConstBuffer(const ConstBufferDecl& cbDecl, void* data = nullptr) override;
	IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) override;
	void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;

	IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr) override;
	void SetVertexBuffer(IVertexBufferPtr vertexBuffer) override;

	bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) override;
	void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) override;
	void SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program);

	IProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	IProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	void SetProgram(IProgramPtr program);

	ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) override;
	void SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count);

	IInputLayoutPtr CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) override;
	void SetVertexLayout(IInputLayoutPtr layout);

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
	ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube);
	VertexShader9Ptr _CreateVS(const char* filename, const char* entry = nullptr);
	PixelShader9Ptr _CreatePS(const char* filename, const char* entry = nullptr);
	VertexShader9Ptr _CreateVSByFXC(const char* filename);
	PixelShader9Ptr _CreatePSByFXC(const char* filename);
	IDirect3DVertexDeclaration9* _CreateInputLayout(Program9* pProgram, const std::vector<D3DVERTEXELEMENT9>& descArr);
private:
	bool _GetDeviceCaps();
	void _SetRasterizerState();
	bool _CreateDeviceAndSwapChain();
};

}