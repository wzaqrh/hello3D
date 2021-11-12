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

	virtual Eigen::Vector4i WinSize() = 0;

	virtual void ClearColorDepthStencil(const Eigen::Vector4f& color, float Depth, unsigned char Stencil) = 0;

	virtual IRenderTexturePtr CreateRenderTexture(int width, int height, DXGI_FORMAT format) = 0;
	virtual void SetRenderTarget(IRenderTexturePtr rendTarget) = 0;

	virtual IIndexBufferPtr CreateIndexBuffer(int bufferSize, DXGI_FORMAT format, void* buffer) = 0;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) = 0;

	virtual IVertexBufferPtr CreateVertexBuffer(int bufferSize, int stride, int offset, void* buffer) = 0;
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer) = 0;
	
	virtual IContantBufferPtr CloneConstBuffer(IContantBufferPtr buffer) = 0;
	virtual IContantBufferPtr CreateConstBuffer(const ConstBufferDecl& cbDecl, void* data) = 0;

	virtual bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) = 0;
	virtual void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) = 0;
	virtual void SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program) = 0;

	virtual IProgramPtr CreateProgramByCompile(const std::string& vsPath, 
		const std::string& psPath, 
		const std::string& vsEntry, 
		const std::string& psEntry) = 0;
	virtual IProgramPtr CreateProgramByFXC(const std::string& name, 
		const std::string& vsEntry, 
		const std::string& psEntry) = 0;
	virtual IProgramPtr CreateProgram(const std::string& name, 
		const std::string& vsEntry, 
		const std::string& psEntry) = 0;
	virtual void SetProgram(IProgramPtr program) = 0;

	virtual ISamplerStatePtr CreateSampler(D3D11_FILTER filter, D3D11_COMPARISON_FUNC comp) = 0;
	virtual void SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count) = 0;
	
	virtual IInputLayoutPtr CreateLayout(IProgramPtr pProgram, D3D11_INPUT_ELEMENT_DESC descArray[], size_t descCount) = 0;
	virtual void SetVertexLayout(IInputLayoutPtr layout) = 0;

	virtual const BlendFunc& GetBlendFunc() const = 0;
	virtual const DepthState& GetDepthState() const = 0;
	virtual void SetBlendFunc(const BlendFunc& blendFunc) = 0;
	virtual void SetDepthState(const DepthState& depthState) = 0;

	virtual ITexturePtr LoadTexture(const std::string& imgPath, DXGI_FORMAT format, bool async, bool isCube)= 0;
	virtual ITexturePtr CreateTexture(int width, int height, DXGI_FORMAT format, int mipmap) = 0;
	virtual bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) = 0;
	virtual void SetTexture(size_t slot, ITexturePtr texture) = 0;
	virtual void SetTextures(size_t slot, ITexturePtr textures[], size_t count) = 0;

	virtual void DrawPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) = 0;
	virtual void DrawIndexedPrimitive(const RenderOperation& op, D3D11_PRIMITIVE_TOPOLOGY topo) = 0;

	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
};

struct __declspec(align(16)) RenderSystem : public IRenderSystem
{
public:
	RenderSystem();
	virtual ~RenderSystem();
public:
	const BlendFunc& GetBlendFunc() const override { return mCurBlendFunc; }
	const DepthState& GetDepthState() const override { return mCurDepthState; }

	Eigen::Vector4i WinSize() { return Eigen::Vector4i{ mScreenWidth, mScreenHeight, 0, 0 }; }
	IProgramPtr CreateProgram(const std::string& name, const std::string& vsEntry, const std::string& psEntry);
public:
	ITexturePtr LoadTexture(const std::string& imgPath, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, 
		bool async = true, bool isCube = false);
protected:
	virtual ITexturePtr _CreateTexture(const char* pSrcFile, DXGI_FORMAT format, bool async, bool isCube) = 0;
public:
	int mScreenWidth, mScreenHeight;

	BlendFunc mCurBlendFunc;
	DepthState mCurDepthState;

	std::map<std::string, ITexturePtr> mTexByPath;
	std::string mFXCDir;
};

}