#pragma once
#include <boost/noncopyable.hpp>
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/renderable/renderable.h"

namespace mir {

interface IRenderSystem : boost::noncopyable 
{
	virtual bool Initialize(HWND hWnd, RECT vp = { 0,0,0,0 }) = 0;
	virtual void Update(float dt) = 0;
	virtual void CleanUp() = 0;
	virtual void SetViewPort(int x, int y, int w, int h) = 0;

	virtual XMINT4 GetWinSize() = 0;

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
	virtual void SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program) = 0;

	virtual IProgramPtr CreateProgramByCompile(const char* vsPath, const char* psPath = nullptr, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual IProgramPtr CreateProgramByFXC(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual IProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr) = 0;
	virtual void SetProgram(IProgramPtr program) = 0;

	virtual ISamplerStatePtr CreateSampler(D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_FUNC comp = D3D11_COMPARISON_NEVER) = 0;
	virtual void SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count) = 0;
	
	virtual IInputLayoutPtr CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC* descArray, size_t descCount) = 0;
	virtual void SetVertexLayout(IInputLayoutPtr layout) = 0;

	virtual const BlendFunc& GetBlendFunc() const = 0;
	virtual const DepthState& GetDepthState() const = 0;
	virtual void SetBlendFunc(const BlendFunc& blendFunc) = 0;
	virtual void SetDepthState(const DepthState& depthState) = 0;

	virtual ITexturePtr LoadTexture(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = true, bool isCube = false)= 0;
	virtual ITexturePtr CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap) = 0;
	virtual bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) = 0;
	virtual void SetTexture(size_t slot, ITexturePtr texture) = 0;
	virtual void SetTextures(size_t slot, ITexturePtr textures[], size_t count) = 0;

	virtual void DrawPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) = 0;
	virtual void DrawIndexedPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) = 0;

	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
};

struct RenderPipeline 
{
	size_t mDrawCount = 0, mDrawLimit = INT_MAX;

	bool mCastShdowFlag = false;
	std::vector<IRenderTexturePtr> mRenderTargetStk;
	IRenderTexturePtr mShadowPassRT, mPostProcessRT;
public:
	int mScreenWidth, mScreenHeight;
	SceneManagerPtr mSceneManager;
	IRenderSystemPtr mRenderSys;
public:
	RenderPipeline(IRenderSystemPtr renderSys,  int width, int height);
	void Draw(IRenderable& renderable);
	bool BeginFrame();
	void EndFrame();
private:
	void _RenderSkyBox();
	void _DoPostProcess();
	void _PushRenderTarget(IRenderTexturePtr rendTarget);
	void _PopRenderTarget();
	void RenderOpQueue(const RenderOperationQueue& opQueue, const std::string& lightMode);
	void MakeAutoParam(cbGlobalParam& param, CameraBase* pLightCam, bool castShadow, cbDirectLight* light, LightType lightType);
	void RenderLight(cbDirectLight* light, LightType lightType, const RenderOperationQueue& opQueue, const std::string& lightMode);
	void RenderOp(const RenderOperation& op, const std::string& lightMode, const cbGlobalParam& globalParam);
	void RenderPass(const PassPtr& pass, TextureBySlot& textures, int iterCnt, const RenderOperation& op, const cbGlobalParam& globalParam);
	void BindPass(const PassPtr& pass, const cbGlobalParam& globalParam);
};
typedef std::shared_ptr<RenderPipeline> RenderPipelinePtr;

struct __declspec(align(16)) RenderSystem : public IRenderSystem
{
public:
	int mScreenWidth, mScreenHeight;

	BlendFunc mCurBlendFunc;
	DepthState mCurDepthState;

	std::map<std::string, ITexturePtr> mTexByPath;
	std::string mFXCDir;
public:
	void* operator new(size_t i){ return _mm_malloc(i,16); }
	void operator delete(void* p) { _mm_free(p); }
	RenderSystem();
	virtual ~RenderSystem();
public:
	const BlendFunc& GetBlendFunc() const override { return mCurBlendFunc; }
	const DepthState& GetDepthState() const override { return mCurDepthState; }

	XMINT4 GetWinSize() { return XMINT4{ mScreenWidth, mScreenHeight, 0, 0 }; }
	IProgramPtr CreateProgram(const std::string& name, const char* vsEntry = nullptr, const char* psEntry = nullptr);
public:
	ITexturePtr LoadTexture(const std::string& __imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, bool async = true, bool isCube = false);
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube) = 0;
};

}