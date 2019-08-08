#pragma once
#include "IRenderSystem.h"

class TRenderSystem9
	: public IRenderSystem
{
	HINSTANCE mHInst = NULL;
	HWND mHWnd = NULL;

	LPDIRECT3D9             g_pD3D = NULL; // Used to create the D3DDevice
	LPDIRECT3DDEVICE9       mDevice9 = NULL; // Our rendering device
	LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; // Buffer to hold vertices
	LPDIRECT3DTEXTURE9      g_pTexture = NULL; // Our texture
public:
	int mScreenWidth;
	int mScreenHeight;
	TD3DInput* mInput = nullptr;
public:
	TSkyBoxPtr mSkyBox;
	TCameraPtr mDefCamera;
public:
	TRenderSystem9();
	virtual ~TRenderSystem9();

	virtual bool Initialize() override;
	virtual void Update(float dt) override;
	virtual void CleanUp() override;
public:
	virtual TSpotLightPtr AddSpotLight() override;
	virtual TPointLightPtr AddPointLight() override;
	virtual TDirectLightPtr AddDirectLight() override;
	virtual TCameraPtr SetCamera(double fov, int eyeDistance, double far1) override;
	virtual TSkyBoxPtr SetSkyBox(const std::string& imgName) override;
	virtual TPostProcessPtr AddPostProcess(const std::string& name) override;
public:
	virtual void SetHandle(HINSTANCE hInstance, HWND hWnd) override;
	virtual void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil) override;

	virtual IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT) override;
	virtual void ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color) override;
	virtual void SetRenderTarget(IRenderTexturePtr rendTarget) override;

	virtual TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback) override;

	virtual IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) override;
	virtual IContantBufferPtr CreateConstBuffer(int bufferSize, void* data = nullptr) override;
	virtual IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) override;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;
	virtual void DrawIndexed(IIndexBufferPtr indexBuffer) override;

	virtual IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr) override;
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer) override;

	virtual bool UpdateBuffer(IHardwareBuffer* buffer, void* data, int dataSize) override;
	virtual void UpdateConstBuffer(IContantBufferPtr buffer, void* data) override;

	virtual TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	virtual TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	virtual TProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;

	virtual ID3D11SamplerState* CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) override;
	virtual IInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) override;

	virtual void SetBlendFunc(const TBlendFunc& blendFunc) override;
	virtual void SetDepthState(const TDepthState& depthState) override;
public:
	virtual bool BeginScene() override;
	virtual void EndScene() override;
	virtual void Draw(IRenderable* renderable) override;
	virtual void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode) override;
protected:
	ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = false);
private:
	void _SetRasterizerState();
	bool _CreateDeviceAndSwapChain();
};
