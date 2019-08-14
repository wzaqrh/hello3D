#pragma once
#include "TBaseTypes.h"
#include "TPredefine.h"

class TD3DInput;
__declspec(align(16)) class IRenderSystem
{
protected:
	std::map<std::string, ITexturePtr> mTexByPath;
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

	virtual bool Initialize() = 0;
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
	virtual void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth, UINT8 Stencil) = 0;

	virtual IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT) = 0;
	virtual void ClearRenderTexture(IRenderTexturePtr rendTarget, XMFLOAT4 color, FLOAT Depth = 1.0, UINT8 Stencil = 0) = 0;
	virtual void SetRenderTarget(IRenderTexturePtr rendTarget) = 0;

	virtual TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback) = 0;

	virtual IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) = 0;
	virtual IContantBufferPtr CreateConstBuffer(int bufferSize, void* data = nullptr) = 0;
	virtual IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) = 0;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) = 0;

	virtual IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr) = 0;
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer) = 0;

	virtual bool UpdateBuffer(IHardwareBuffer* buffer, void* data, int dataSize) = 0;
	virtual void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) = 0;

	virtual TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	TProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);

	virtual ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) = 0;
	virtual IInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) = 0;

	virtual void SetBlendFunc(const TBlendFunc& blendFunc) = 0;
	virtual void SetDepthState(const TDepthState& depthState) = 0;
public:
	ITexturePtr GetTexByPath(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);
public:
	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void Draw(IRenderable* renderable) = 0;
	virtual void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode) = 0;
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = false) = 0;
};
