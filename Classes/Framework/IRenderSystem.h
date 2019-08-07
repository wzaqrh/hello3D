#pragma once
#include "TBaseTypes.h"
#include "TPredefine.h"

class TD3DInput;
__declspec(align(16)) class IRenderSystem
{

public:
	int mScreenWidth;
	int mScreenHeight;
	TD3DInput* mInput = nullptr;
public:
	TSkyBoxPtr mSkyBox;
	TCameraPtr mDefCamera;
public:
	IRenderSystem();
	virtual ~IRenderSystem();

	virtual HRESULT Initialize() = 0;
	virtual void Update(float dt) = 0;
	virtual void CleanUp() = 0;
public:
	virtual TSpotLightPtr AddSpotLight() = 0;
	virtual TPointLightPtr AddPointLight() = 0;
	virtual TDirectLightPtr AddDirectLight() = 0;
	virtual TCameraPtr SetCamera(double fov, int eyeDistance, double far1) = 0;
	virtual TSkyBoxPtr SetSkyBox(const std::string& imgName) = 0;
	virtual TPostProcessPtr AddPostProcess(const std::string& name) = 0;
public:
	virtual void SetHandle(HINSTANCE hInstance, HWND hWnd) = 0;
	virtual void ClearColor(const XMFLOAT4& color) = 0;
	virtual void ClearDepthStencil(FLOAT Depth, UINT8 Stencil) = 0;

	virtual TRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT) = 0;
	virtual void ClearRenderTexture(TRenderTexturePtr rendTarget, XMFLOAT4 color) = 0;
	virtual void SetRenderTarget(TRenderTexturePtr rendTarget) = 0;

	virtual TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback) = 0;

	virtual TContantBufferPtr CloneConstBuffer(TContantBufferPtr buffer) = 0;
	virtual TContantBufferPtr CreateConstBuffer(int bufferSize, void* data = nullptr) = 0;
	virtual TIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) = 0;
	virtual void SetIndexBuffer(TIndexBufferPtr indexBuffer) = 0;
	virtual void DrawIndexed(TIndexBufferPtr indexBuffer) = 0;

	virtual TVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr) = 0;
	virtual void SetVertexBuffer(TVertexBufferPtr vertexBuffer) = 0;

	virtual bool UpdateBuffer(THardwareBuffer* buffer, void* data, int dataSize) = 0;
	virtual void UpdateConstBuffer(TContantBufferPtr buffer, void* data) = 0;

	virtual TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual TProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;

	virtual ID3D11SamplerState* CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) = 0;
	virtual TInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) = 0;

	virtual TTexturePtr GetTexByPath(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN) = 0;

	virtual void SetBlendFunc(const TBlendFunc& blendFunc) = 0;
	virtual void SetDepthState(const TDepthState& depthState) = 0;
public:
	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void Draw(IRenderable* renderable) = 0;
	virtual void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode) = 0;
};