#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/interface_type.h"
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

	virtual Eigen::Vector2i WinSize() const = 0;

	virtual void ClearColorDepthStencil(const Eigen::Vector4f& color, float Depth, uint8_t Stencil) = 0;

	virtual IResourcePtr CreateResource(DeviceResourceType deviceResType) = 0;

	virtual IIndexBufferPtr LoadIndexBuffer(IResourcePtr res, int bufferSize, ResourceFormat format, void* buffer) = 0;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) = 0;

	virtual IVertexBufferPtr LoadVertexBuffer(IResourcePtr res, int bufSize, int stride, int offset, void* buffer) = 0;
	virtual void SetVertexBuffer(IVertexBufferPtr vertexBuffer) = 0;
	
	virtual IContantBufferPtr LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, void* data) = 0;
	virtual bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) = 0;
	virtual void SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program) = 0;

	virtual IBlobDataPtr CompileShader(const ShaderCompileDesc& desc, const Data& data) = 0;
	virtual IShaderPtr CreateShader(ShaderType type, const ShaderCompileDesc& desc, IBlobDataPtr data) = 0;
	virtual IProgramPtr LoadProgram(IResourcePtr res, const std::vector<IShaderPtr>& shaders) = 0;
	virtual void SetProgram(IProgramPtr program) = 0;
	
	virtual IInputLayoutPtr LoadLayout(IResourcePtr res, IProgramPtr pProgram, const std::vector<LayoutInputElement>& descArr) = 0;
	virtual void SetVertexLayout(IInputLayoutPtr layout) = 0;

	virtual ISamplerStatePtr LoadSampler(IResourcePtr res, SamplerFilterMode filter, CompareFunc comp) = 0;
	virtual void SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count) = 0;

	virtual ITexturePtr LoadTexture(IResourcePtr res, ResourceFormat format, 
		const Eigen::Vector4i& w_h_step_face, int mipmap, const Data datas[]) = 0;
	virtual bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) = 0;
	virtual void SetTexture(size_t slot, ITexturePtr texture) = 0;
	virtual void SetTextures(size_t slot, ITexturePtr textures[], size_t count) = 0;

	virtual IRenderTexturePtr LoadRenderTexture(IResourcePtr res, const Eigen::Vector2i& size, ResourceFormat format) = 0;
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
	Eigen::Vector2i WinSize() const override { return mScreenSize; }
	const BlendState& GetBlendFunc() const override { return mCurBlendFunc; }
	const DepthState& GetDepthState() const override { return mCurDepthState; }
protected:
	Eigen::Vector2i mScreenSize;
	BlendState mCurBlendFunc;
	DepthState mCurDepthState;
};

}