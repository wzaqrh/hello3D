#pragma once
#include "IRenderSystem.h"

class TRenderSystem9
	: public IRenderSystem
{
	HINSTANCE mHInst = NULL;
	HWND mHWnd = NULL;

	IDirect3D9 *mD3D9 = NULL; // Used to create the D3DDevice
	IDirect3DDevice9 *mDevice9 = NULL; // Our rendering device

	IDirect3DSurface9 *mBackColorBuffer = NULL, *mBackDepthStencilBuffer = NULL;
	IDirect3DSurface9 *mCurColorBuffer = NULL, *mCurDepthStencilBuffer = NULL;
public:
	TRenderSystem9();
	virtual ~TRenderSystem9();

	virtual bool Initialize() override;
	virtual void Update(float dt) override;
	virtual void CleanUp() override;
public:
	virtual void SetHandle(HINSTANCE hInstance, HWND hWnd) override;
	virtual void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil) override;

	virtual IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT) override;
	virtual void SetRenderTarget(IRenderTexturePtr rendTarget) override;

	virtual TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback) override;

	virtual IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) override;
	virtual IContantBufferPtr CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data = nullptr) override;
	virtual IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) override;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;

	virtual IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr) override;
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer) override;

	virtual bool UpdateBuffer(IHardwareBuffer* buffer, void* data, int dataSize) override;
	virtual void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) override;

	virtual TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	virtual TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;

	virtual ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) override;
	virtual IInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) override;

	virtual void SetBlendFunc(const TBlendFunc& blendFunc) override;
	virtual void SetDepthState(const TDepthState& depthState) override;
public:
	virtual bool BeginScene() override;
	virtual void EndScene() override;
	virtual void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode) override;
protected:
	void BindPass(TPassPtr pass, const cbGlobalParam& globalParam);
	void RenderPass(TPassPtr pass, TTextureBySlot& textures, int iterCnt, IIndexBufferPtr indexBuffer, IVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam);
	void RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode);

	ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = false);

	IVertexShaderPtr _CreateVS(const char* filename, const char* entry = nullptr);
	IPixelShaderPtr _CreatePS(const char* filename, const char* entry = nullptr);
	IVertexShaderPtr _CreateVSByFXC(const char* filename);
	IPixelShaderPtr _CreatePSByFXC(const char* filename);

	IDirect3DVertexDeclaration9* _CreateInputLayout(TProgram* pProgram, const std::vector<D3DVERTEXELEMENT9>& descArr);
private:
	void _SetRasterizerState();
	bool _CreateDeviceAndSwapChain();
};
