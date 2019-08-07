#pragma once
#include "IRenderSystem.h"

class TRenderSystem9
	: public IRenderSystem
{
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

	virtual HRESULT Initialize() override;
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
	virtual void ClearColor(const XMFLOAT4& color) override;
	virtual void ClearDepthStencil(FLOAT Depth, UINT8 Stencil) override;

	virtual TRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT) override;
	virtual void ClearRenderTexture(TRenderTexturePtr rendTarget, XMFLOAT4 color) override;
	virtual void SetRenderTarget(TRenderTexturePtr rendTarget) override;

	virtual TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback) override;

	virtual TContantBufferPtr CloneConstBuffer(TContantBufferPtr buffer) override;
	virtual TContantBufferPtr CreateConstBuffer(int bufferSize, void* data = nullptr) override;
	virtual TIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) override;
	virtual void SetIndexBuffer(TIndexBufferPtr indexBuffer) override;
	virtual void DrawIndexed(TIndexBufferPtr indexBuffer) override;

	virtual TVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr) override;
	virtual void SetVertexBuffer(TVertexBufferPtr vertexBuffer) override;

	virtual bool UpdateBuffer(THardwareBuffer* buffer, void* data, int dataSize) override;
	virtual void UpdateConstBuffer(TContantBufferPtr buffer, void* data) override;

	virtual TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	virtual TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;
	virtual TProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) override;

	virtual ID3D11SamplerState* CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) override;
	virtual TInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) override;

	virtual TTexturePtr GetTexByPath(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN) override;

	virtual void SetBlendFunc(const TBlendFunc& blendFunc) override;
	virtual void SetDepthState(const TDepthState& depthState) override;
public:
	virtual bool BeginScene() override;
	virtual void EndScene() override;
	virtual void Draw(IRenderable* renderable) override;
	virtual void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode) override;
};
