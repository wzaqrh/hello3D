#pragma once
#include "core/rendersys/interface_type_pred.h"
#include "core/rendersys/scene_manager_pred.h"
#include "core/rendersys/material_pred.h"
#include "core/rendersys/base_type.h"
#include "core/renderable/renderable.h"

namespace mir {

struct RenderOperation;
struct RenderOperationQueue;
struct IRenderable;

interface IRenderSystem 
{
	virtual bool Initialize(HWND hWnd, RECT vp = { 0,0,0,0 }) = 0;
	virtual void Update(float dt) = 0;
	virtual void CleanUp() = 0;
	virtual void SetViewPort(int x, int y, int w, int h) = 0;

	virtual XMINT4 GetWinSize() = 0;

	virtual SceneManagerPtr GetSceneManager() = 0;

	virtual void ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth = 1.0, UINT8 Stencil = 0) = 0;

	virtual IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT) = 0;
	virtual void SetRenderTarget(IRenderTexturePtr rendTarget) = 0;

	virtual IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) = 0;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) = 0;

	virtual IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer = nullptr) = 0;
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer) = 0;
	
	virtual IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) = 0;
	virtual IContantBufferPtr CreateConstBuffer(const ConstBufferDecl& cbDecl, void* data = nullptr) = 0;

	virtual bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) = 0;
	virtual void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) = 0;

	virtual IProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual IProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual IProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;

	virtual ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) = 0;
	virtual IInputLayoutPtr CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) = 0;

	virtual void SetBlendFunc(const BlendFunc& blendFunc) = 0;
	virtual void SetDepthState(const DepthState& depthState) = 0;

	virtual ITexturePtr LoadTexture(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = true, bool isCube = false)= 0;
	virtual ITexturePtr CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap) = 0;
	virtual bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) = 0;

	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
	virtual void RenderQueue(const RenderOperationQueue& opQueue, const std::string& lightMode) = 0;
	virtual void Draw(IRenderable* renderable) = 0;
};
typedef std::shared_ptr<IRenderSystem> IRenderSystemPtr;

struct __declspec(align(16)) RenderSystem : IRenderSystem
{
protected:
public:
	size_t mDrawCount = 0, mDrawLimit = INT_MAX;
	int mScreenWidth, mScreenHeight;

	std::map<std::string, ITexturePtr> mTexByPath;
	std::string mFXCDir;
	
	BlendFunc mCurBlendFunc;
	DepthState mCurDepthState;

	bool mCastShdowFlag = false;
	std::vector<IRenderTexturePtr> mRenderTargetStk;
	IRenderTexturePtr mShadowPassRT, mPostProcessRT;
public:
	//MaterialFactoryPtr mMaterialFac;
	SceneManagerPtr mSceneManager;
public:
	void* operator new(size_t i){ return _mm_malloc(i,16); }
	void operator delete(void* p) { _mm_free(p); }
	RenderSystem();
	virtual ~RenderSystem();
protected:
	bool _CanDraw();
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();
	void MakeAutoParam(cbGlobalParam& param, CameraBase* pLightCam, bool castShadow, cbDirectLight* light, LightType lightType);
public:
	XMINT4 GetWinSize() {
		XMINT4 ret = { mScreenWidth, mScreenHeight, 0, 0 };
		return ret;
	}
	SceneManagerPtr GetSceneManager();
	IProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);
public:
	ITexturePtr LoadTexture(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = true, bool isCube = false);
public:
	void Draw(IRenderable* renderable);
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube) = 0;
};
typedef std::shared_ptr<RenderSystem> RenderSystemPtr;

}