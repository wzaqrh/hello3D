#pragma once
#include "core/rendersys/interface_type_pred.h"
#include "core/renderable/renderable_pred.h"
#include "core/rendersys/scene_manager_pred.h"
#include "core/rendersys/material_pred.h"
#include "core/rendersys/base_type.h"

namespace mir {

MIDL_INTERFACE("B43DD74F-6C65-4C17-85C1-F89A9B2348AD")
IRenderSystem : public IUnknown
{
	virtual STDMETHODIMP_(bool) Initialize(HWND hWnd, RECT vp = {0,0,0,0}) = 0;
	virtual STDMETHODIMP_(void) Update(float dt) = 0;
	virtual STDMETHODIMP_(void) CleanUp() = 0;
	virtual STDMETHODIMP_(void) SetViewPort(int x, int y, int w, int h) = 0;

	virtual STDMETHODIMP_(XMINT4) GetWinSize() = 0;

	virtual STDMETHODIMP_(ISceneManagerPtr) GetSceneManager() = 0;

	virtual STDMETHODIMP_(void) ClearColorDepthStencil(const XMFLOAT4& color, FLOAT Depth = 1.0, UINT8 Stencil = 0) = 0;

	virtual STDMETHODIMP_(IRenderTexturePtr) CreateRenderTexture(int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT) = 0;
	virtual STDMETHODIMP_(void) SetRenderTarget(IRenderTexturePtr rendTarget) = 0;

	virtual STDMETHODIMP_(TMaterialPtr) CreateMaterial(std::string name, std::function<void(TMaterialPtr material)> callback) = 0;

	virtual STDMETHODIMP_(IIndexBufferPtr) CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) = 0;
	virtual STDMETHODIMP_(void) SetIndexBuffer(IIndexBufferPtr indexBuffer) = 0;

	virtual STDMETHODIMP_(IVertexBufferPtr) CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer = nullptr) = 0;
	virtual STDMETHODIMP_(void) SetVertexBuffer(IVertexBufferPtr vertexBuffer) = 0;
	
	virtual STDMETHODIMP_(IContantBufferPtr) CloneConstBuffer(IContantBufferPtr buffer) = 0;
	virtual STDMETHODIMP_(IContantBufferPtr) CreateConstBuffer(const TConstBufferDecl& cbDecl, void* data = nullptr) = 0;

	virtual STDMETHODIMP_(bool) UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) = 0;
	virtual STDMETHODIMP_(void) UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) = 0;

	virtual STDMETHODIMP_(IProgramPtr) CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual STDMETHODIMP_(IProgramPtr) CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual STDMETHODIMP_(IProgramPtr) CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;

	virtual STDMETHODIMP_(ISamplerStatePtr) CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) = 0;
	virtual STDMETHODIMP_(IInputLayoutPtr) CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) = 0;

	virtual STDMETHODIMP_(void) SetBlendFunc(const TBlendFunc& blendFunc) = 0;
	virtual STDMETHODIMP_(void) SetDepthState(const TDepthState& depthState) = 0;

	virtual STDMETHODIMP_(ITexturePtr) LoadTexture(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = true, bool isCube = false)= 0;
	virtual STDMETHODIMP_(ITexturePtr) CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap) = 0;
	virtual STDMETHODIMP_(bool) LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) = 0;

	virtual STDMETHODIMP_(bool) BeginScene() = 0;
	virtual STDMETHODIMP_(void) EndScene() = 0;
	virtual STDMETHODIMP_(void) RenderQueue(const TRenderOperationQueue& opQueue, const std::string& lightMode) = 0;
	virtual STDMETHODIMP_(void) Draw(IRenderable* renderable) = 0;
};

struct INHERIT_COM("BF1920DB-54DB-42D2-AAA5-8E2F91482B7B")
__declspec(align(16)) 
TRenderSystem : ComBase<IRenderSystem>
{
protected:
	size_t mDrawCount = 0, mDrawLimit = INT_MAX;
	int mScreenWidth, mScreenHeight;

	std::map<std::string, ITexturePtr> mTexByPath;
	std::string mFXCDir;
	
	TBlendFunc mCurBlendFunc;
	TDepthState mCurDepthState;

	bool mCastShdowFlag = false;
	std::vector<IRenderTexturePtr> mRenderTargetStk;
	IRenderTexturePtr mShadowPassRT, mPostProcessRT;

	TMaterialFactoryPtr mMaterialFac;
	TSceneManagerPtr mSceneManager;
public:
	void* operator new(size_t i){ return _mm_malloc(i,16); }
	void operator delete(void* p) { _mm_free(p); }
	TRenderSystem();
	virtual ~TRenderSystem();
protected:
	bool _CanDraw();
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();
	void MakeAutoParam(cbGlobalParam& param, TCameraBase* pLightCam, bool castShadow, TDirectLight* light, enLightType lightType);
public:
	STDMETHODIMP_(XMINT4) GetWinSize() {
		XMINT4 ret = { mScreenWidth, mScreenHeight, 0, 0 };
		return ret;
	}
	STDMETHODIMP_(ISceneManagerPtr) GetSceneManager();
	STDMETHODIMP_(IProgramPtr) CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);
public:
	STDMETHODIMP_(ITexturePtr) LoadTexture(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = true, bool isCube = false);
public:
	STDMETHODIMP_(void) Draw(IRenderable* renderable);
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube) = 0;
};

}