#pragma once
#include "TBaseTypes.h"
#include "TPredefine.h"

class TD3DInput;
__declspec(align(16)) class TRenderSystem
{
public:
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
	int mScreenWidth;
	int mScreenHeight;
	TD3DInput* mInput = nullptr;
	TMaterialFactoryPtr mMaterialFac;
	std::map<std::string, TTexturePtr> mTexByPath;
	TRenderTexturePtr mShadowPassRT,mPostProcessRT;

	ID3D11RenderTargetView* mBackRenderTargetView = NULL;
	ID3D11DepthStencilView* mBackDepthStencilView = NULL;
	ID3D11RenderTargetView* mCurRenderTargetView = NULL;
	ID3D11DepthStencilView* mCurDepthStencilView = NULL;
	std::vector<TRenderTexturePtr> mRenderTargetStk;

	TThreadPumpPtr mThreadPump;

	bool mCastShdowFlag = false;
	TBlendFunc mCurBlendFunc;
	TDepthState mCurDepthState;
public:
	TSkyBoxPtr mSkyBox;
	TCameraPtr mDefCamera;
	std::vector<TPostProcessPtr> mPostProcs;
	std::vector<TPointLightPtr> mPointLights;
	std::vector<TDirectLightPtr> mDirectLights;
	std::vector<TSpotLightPtr> mSpotLights;
	std::vector<std::pair<TDirectLight*, enLightType>> mLightsOrder;
public:
	void* operator new(size_t i){
		return _mm_malloc(i,16);
	}
	void operator delete(void* p) {
		_mm_free(p);
	}
	TRenderSystem();
	~TRenderSystem();

	HRESULT Initialize();
	void Update(float dt);
	void CleanUp();
public:
	TSpotLightPtr AddSpotLight();
	TPointLightPtr AddPointLight();
	TDirectLightPtr AddDirectLight();
	TCameraPtr SetCamera(double fov, int eyeDistance, double far1);
	TSkyBoxPtr SetSkyBox(const std::string& imgName);
	TPostProcessPtr AddPostProcess(const std::string& name);
public:
	TRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format=DXGI_FORMAT_R32G32B32A32_FLOAT);
	void ClearRenderTexture(TRenderTexturePtr rendTarget, XMFLOAT4 color);
	void _SetRenderTarget(TRenderTexturePtr rendTarget);
	void _PushRenderTarget(TRenderTexturePtr rendTarget);
	void _PopRenderTarget();

	TMaterialPtr CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback);
	//TMaterialPtr CreateMaterial(const char* vsPath, const char* psPath, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);

	TContantBufferPtr CloneConstBuffer(TContantBufferPtr buffer);
	TContantBufferPtr CreateConstBuffer(int bufferSize, void* data = nullptr);
	TIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer);
	void SetIndexBuffer(TIndexBufferPtr indexBuffer);
	void DrawIndexed(TIndexBufferPtr indexBuffer);

	ID3D11Buffer* _CreateVertexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize);
	TVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer=nullptr);
	void SetVertexBuffer(TVertexBufferPtr vertexBuffer);

	bool UpdateBuffer(THardwareBuffer* buffer, void* data, int dataSize);
	void UpdateConstBuffer(TContantBufferPtr buffer, void* data);

	TVertexShaderPtr _CreateVS(const char* filename, const char* entry = nullptr, bool async = true);
	TVertexShaderPtr _CreateVSByFXC(const char* filename);
	TPixelShaderPtr _CreatePS(const char* filename, const char* entry = nullptr, bool async = true);
	TPixelShaderPtr _CreatePSByFXC(const char* filename);
	TProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr);
	TProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);
	TProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);

	ID3D11SamplerState* CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER);
	TInputLayoutPtr CreateLayout(TProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount);
	ID3D11InputLayout* _CreateInputLayout(TProgram* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr);

	TTexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format=DXGI_FORMAT_UNKNOWN, bool async=false);
	TTexturePtr GetTexByPath(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);

	void SetBlendFunc(const TBlendFunc& blendFunc);
	void SetDepthState(const TDepthState& depthState);
public:
	bool BeginScene();
	void EndScene();
	void Draw(IRenderable* renderable);
	void RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode);
private:
	void RenderLight(TDirectLight* light, enLightType lightType, const TRenderOperationQueue& opQueue, const std::string& lightMode);
	void RenderOperation(const TRenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderPass(TPassPtr pass, TTextureBySlot& texturs, int iterCnt, TIndexBufferPtr indexBuffer, TVertexBufferPtr vertexBuffer, const cbGlobalParam& globalParam);
	void RenderSkyBox();
	void DoPostProcess();

	cbGlobalParam MakeAutoParam(TCameraBase* pLightCam, bool castShadow, TDirectLight* light, enLightType lightType);
	void BindPass(TPassPtr pass, const cbGlobalParam& globalParam);
};
