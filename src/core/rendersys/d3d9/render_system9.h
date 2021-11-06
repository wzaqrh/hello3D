#pragma once
#include "core/rendersys/render_system.h"
#include "interface_type9.h"

namespace mir {

class TRenderSystem9 : public TRenderSystem
{
	HWND mHWnd = NULL;

	D3DCAPS9 mD3DCaps;
	IDirect3D9 *mD3D9 = NULL; // Used to create the D3DDevice
	IDirect3DDevice9 *mDevice9 = NULL; // Our rendering device

	IDirect3DSurface9 *mBackColorBuffer = NULL, *mBackDepthStencilBuffer = NULL;
	IDirect3DSurface9 *mCurColorBuffer = NULL, *mCurDepthStencilBuffer = NULL;

	std::vector<D3DXMACRO> mShaderMacros;
public:
	TRenderSystem9();
	virtual ~TRenderSystem9();

	bool Initialize(HWND hWnd, RECT vp) override;
	void Update(float dt) override;
	void CleanUp() override;
	void SetViewPort(int x, int y, int w, int h);
public:
	void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil) override;

	IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT) override;
	void SetRenderTarget(IRenderTexturePtr rendTarget) override;

	TMaterialPtr GetMaterial(const std::string& name, bool sharedUse) override;

	IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) override;
	IContantBufferPtr CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data = nullptr) override;
	IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) override;
	void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;

	IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr) override;
	void SetVertexBuffer(IVertexBufferPtr vertexBuffer) override;

	bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) override;
	void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) override;

	IProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	IProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;

	ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) override;
	IInputLayoutPtr CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) override;

	void SetBlendFunc(const TBlendFunc& blendFunc) override;
	void SetDepthState(const TDepthState& depthState) override;

	ITexturePtr CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap) override;
	bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) override;
public:
	bool BeginScene() override;
	void EndScene() override;
	void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode) override;
protected:
	void BindPass(TPassPtr pass, const cbGlobalParam& globalParam);
	void RenderPass(TPassPtr pass, TTextureBySlot& texturs, int iterCnt, IIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam);
	void RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode);
	void _RenderSkyBox();
	void _DoPostProcess();

	ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube);
	TVertexShader9Ptr _CreateVS(const char* filename, const char* entry = nullptr);
	TPixelShader9Ptr _CreatePS(const char* filename, const char* entry = nullptr);
	TVertexShader9Ptr _CreateVSByFXC(const char* filename);
	TPixelShader9Ptr _CreatePSByFXC(const char* filename);
	IDirect3DVertexDeclaration9* _CreateInputLayout(TProgram9* pProgram, const std::vector<D3DVERTEXELEMENT9>& descArr);
private:
	bool _GetDeviceCaps();
	void _SetRasterizerState();
	bool _CreateDeviceAndSwapChain();
};

}