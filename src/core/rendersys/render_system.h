#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/renderable/renderable.h"

namespace mir {

enum DeviceResourceType {
	kDeviceResourceInputLayout,
	kDeviceResourceProgram,
	kDeviceResourceVertexBuffer,
	kDeviceResourceIndexBuffer,
	kDeviceResourceContantBuffer,
	kDeviceResourceTexture,
	kDeviceResourceRenderTexture,
	kDeviceResourceSamplerState
};

interface MIR_CORE_API IRenderSystem : boost::noncopyable 
{
	virtual bool Initialize(HWND hWnd, RECT vp = { 0,0,0,0 }) = 0;
	virtual void Update(float dt) = 0;
	virtual void CleanUp() = 0;
	virtual void SetViewPort(int x, int y, int w, int h) = 0;

	virtual Eigen::Vector4i WinSize() = 0;

	virtual void ClearColorDepthStencil(const Eigen::Vector4f& color, float Depth, unsigned char Stencil) = 0;

	virtual IResourcePtr CreateResource(DeviceResourceType deviceResType) = 0;

	virtual IIndexBufferPtr LoadIndexBuffer(IResourcePtr res, int bufferSize, ResourceFormat format, void* buffer) = 0;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) = 0;

	virtual IVertexBufferPtr LoadVertexBuffer(IResourcePtr res, int bufferSize, int stride, int offset, void* buffer) = 0;
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer) = 0;
	
	virtual IContantBufferPtr LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, void* data) = 0;
	virtual bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) = 0;
	virtual void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) = 0;
	virtual void SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program) = 0;

	virtual IProgramPtr LoadProgram(IResourcePtr res, const std::string& name,
		const std::string& vsEntry, const std::string& psEntry) = 0;
	virtual void SetProgram(IProgramPtr program) = 0;
	
	virtual IInputLayoutPtr LoadLayout(IResourcePtr res, IProgramPtr pProgram, const LayoutInputElement descArray[], size_t descCount) = 0;
	virtual void SetVertexLayout(IInputLayoutPtr layout) = 0;

	virtual ISamplerStatePtr LoadSampler(IResourcePtr res, SamplerFilterMode filter, CompareFunc comp) = 0;
	virtual void SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count) = 0;

	virtual ITexturePtr LoadTexture(IResourcePtr res, int width, int height, ResourceFormat format, int mipmap, void* data) = 0;
	virtual ITexturePtr LoadTexture(IResourcePtr res, const std::string& imgPath, ResourceFormat format, bool async, bool isCube) = 0;
	virtual bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) = 0;
	virtual void SetTexture(size_t slot, ITexturePtr texture) = 0;
	virtual void SetTextures(size_t slot, ITexturePtr textures[], size_t count) = 0;

	virtual IRenderTexturePtr LoadRenderTexture(IResourcePtr res, int width, int height, ResourceFormat format) = 0;
	virtual void SetRenderTarget(IRenderTexturePtr rendTarget) = 0;

	virtual const BlendState& GetBlendFunc() const = 0;
	virtual const DepthState& GetDepthState() const = 0;
	virtual void SetBlendFunc(const BlendState& blendFunc) = 0;
	virtual void SetDepthState(const DepthState& depthState) = 0;

	virtual void DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) = 0;
	virtual void DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) = 0;

	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
};

struct MIR_CORE_API RenderSystem : public IRenderSystem
{
public:
	RenderSystem();
	virtual ~RenderSystem();
public:
	Eigen::Vector4i WinSize() { return Eigen::Vector4i{ mScreenWidth, mScreenHeight, 0, 0 }; }
	const BlendState& GetBlendFunc() const override { return mCurBlendFunc; }
	const DepthState& GetDepthState() const override { return mCurDepthState; }
	
	IProgramPtr LoadProgram(IResourcePtr res, const std::string& name, 
		const std::string& vsEntry, 
		const std::string& psEntry) override final;
	ITexturePtr LoadTexture(IResourcePtr res, int width, int height, ResourceFormat format, int mipmap, void* data) override {
		return nullptr;
	}
	ITexturePtr LoadTexture(IResourcePtr res, const std::string& imgPath, ResourceFormat format = kFormatUnknown, 
		bool async = true, bool isCube = false) override final;
protected:
	virtual ITexturePtr _CreateTexture(IResourcePtr res, const char* pSrcFile, ResourceFormat format, 
		bool async, bool isCube) = 0;
	virtual IProgramPtr CreateProgramByCompile(IResourcePtr res, const std::string& vsPath,
		const std::string& psPath,
		const std::string& vsEntry,
		const std::string& psEntry) = 0;
	virtual IProgramPtr CreateProgramByFXC(IResourcePtr res, const std::string& name,
		const std::string& vsEntry,
		const std::string& psEntry) = 0;
public:
	int mScreenWidth, mScreenHeight;

	BlendState mCurBlendFunc;
	DepthState mCurDepthState;

	std::map<std::string, ITexturePtr> mTexByPath;
	std::string mFXCDir;
};

}