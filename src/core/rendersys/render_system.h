#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/data.h"
#include "core/base/base_type.h"
#include "core/rendersys/hardware_buffer.h"
#include "core/rendersys/program.h"
#include "core/rendersys/sampler.h"
#include "core/rendersys/input_layout.h"

namespace mir {

enum DeviceResourceType {
	kDeviceResourceInputLayout,
	kDeviceResourceProgram,
	kDeviceResourceVertexBuffer,
	kDeviceResourceIndexBuffer,
	kDeviceResourceContantBuffer,
	kDeviceResourceTexture,
	kDeviceResourceFrameBuffer,
	kDeviceResourceSamplerState
};

interface MIR_CORE_API IRenderSystem : boost::noncopyable 
{
	virtual ~IRenderSystem() {}
	virtual bool Initialize(HWND hWnd, RECT vp = { 0,0,0,0 }) = 0;
	virtual void Update(float dt) = 0;
	virtual void Dispose() = 0;
	
	/***** query *****/
	virtual std::string GetPlatform() const = 0;
	virtual Eigen::Vector2i WinSize() const = 0;
	
	/***** about resource *****/
	virtual IResourcePtr CreateResource(DeviceResourceType deviceResType) = 0;

	//last formats is zstencil attachment
	virtual IFrameBufferPtr LoadFrameBuffer(IResourcePtr res, const Eigen::Vector2i& size, const std::vector<ResourceFormat>& formats) = 0;
	IFrameBufferPtr LoadFrameBuffer(IResourcePtr res, const Eigen::Vector2i& size, ResourceFormat attach0Format) {
		return LoadFrameBuffer(res, size, std::vector<ResourceFormat>{attach0Format});
	}
	virtual void SetFrameBuffer(IFrameBufferPtr rendTarget) = 0;
	virtual void ClearFrameBuffer(IFrameBufferPtr rendTarget, const Eigen::Vector4f& color, float Depth, uint8_t Stencil) = 0;

	virtual IIndexBufferPtr LoadIndexBuffer(IResourcePtr res, ResourceFormat format, const Data& data) = 0;
	virtual void SetIndexBuffer(IIndexBufferPtr indexBuffer) = 0;

	virtual IVertexBufferPtr LoadVertexBuffer(IResourcePtr res, int stride, int offset, const Data& data) = 0;
	virtual void SetVertexBuffers(size_t slot, const IVertexBufferPtr vertexBuffers[], size_t count) = 0;
	void SetVertexBuffers(const std::vector<IVertexBufferPtr>& vertexBuffers, size_t slot = 0) { 
		SetVertexBuffers(slot, &vertexBuffers[0], vertexBuffers.size()); 
	}
	void SetVertexBuffer(IVertexBufferPtr vertexBuffer, size_t slot = 0) { 
		SetVertexBuffers(slot, &vertexBuffer, 1); 
	}

	virtual IContantBufferPtr LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, HWMemoryUsage usage, const Data& data) = 0;
	virtual void SetConstBuffers(size_t slot, const IContantBufferPtr buffers[], size_t count, IProgramPtr program) = 0;
	void SetConstBuffers(const std::vector<IContantBufferPtr>& constBuffers, IProgramPtr program, size_t slot = 0) {
		SetConstBuffers(slot, !constBuffers.empty() ? &constBuffers[0] : nullptr, constBuffers.size(), program);
	}
	void SetConstBuffer(IContantBufferPtr constBuffer, IProgramPtr program, size_t slot = 0) {
		SetConstBuffers(slot, &constBuffer, 1, program);
	}
	virtual bool UpdateBuffer(IHardwareBufferPtr buffer, const Data& data) = 0;

	virtual IBlobDataPtr CompileShader(const ShaderCompileDesc& desc, const Data& data) = 0;
	virtual IShaderPtr CreateShader(int shaderType, IBlobDataPtr data) = 0;
	virtual IProgramPtr LoadProgram(IResourcePtr res, const std::vector<IShaderPtr>& shaders) = 0;
	virtual void SetProgram(IProgramPtr program) = 0;
	
	virtual IInputLayoutPtr LoadLayout(IResourcePtr res, IProgramPtr pProgram, const std::vector<LayoutInputElement>& descArr) = 0;
	virtual void SetVertexLayout(IInputLayoutPtr layout) = 0;

	virtual ISamplerStatePtr LoadSampler(IResourcePtr res, const SamplerDesc& samplerDesc) = 0;
	virtual void SetSamplers(size_t slot, const ISamplerStatePtr samplers[], size_t count) = 0;

	virtual ITexturePtr LoadTexture(IResourcePtr res, ResourceFormat format, const Eigen::Vector4i& w_h_step_face, int mipmap, const Data datas[]) = 0;
	virtual void SetTextures(size_t slot, const ITexturePtr textures[], size_t count) = 0;
	void SetTexture(size_t slot, ITexturePtr texture) { SetTextures(slot, &texture, 1); }
	virtual bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) = 0;

	/***** about state *****/
	virtual void SetViewPort(int x, int y, int w, int h) = 0;

	virtual const BlendState& GetBlendState() const = 0;
	virtual void SetBlendState(const BlendState& blendFunc) = 0;

	virtual const DepthState& GetDepthState() const = 0;
	virtual void SetDepthState(const DepthState& depthState) = 0;

	/***** about draw *****/
	virtual void DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) = 0;
	virtual void DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) = 0;

	virtual bool BeginScene() = 0;
	virtual void EndScene() = 0;
};

struct MIR_CORE_API RenderSystem : public IRenderSystem
{
public:
	float WinWidth() const { return mScreenSize.x(); }
	float WinHeight() const { return mScreenSize.y(); }
	Eigen::Vector2i WinSize() const override { return mScreenSize; }
	const BlendState& GetBlendState() const override { return mCurBlendState; }
	const DepthState& GetDepthState() const override { return mCurDepthState; }
protected:
	Eigen::Vector2i mScreenSize;
	BlendState mCurBlendState;
	DepthState mCurDepthState;
};

}