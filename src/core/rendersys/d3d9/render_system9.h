#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "core/rendersys/d3d9/predeclare.h"
#include "core/rendersys/render_system.h"

namespace mir {

class RenderSystem9 : public RenderSystem
{
	HWND mHWnd = NULL;

	D3DCAPS9 mD3DCaps;
	IDirect3D9 *mD3D9 = NULL; // Used to create the D3DDevice
	IDirect3DDevice9 *mDevice9 = NULL; // Our rendering device

	IDirect3DSurface9 *mBackColorBuffer = NULL, *mBackDepthStencilBuffer = NULL;
	IDirect3DSurface9 *mCurColorBuffer = NULL, *mCurDepthStencilBuffer = NULL;

	std::vector<D3DXMACRO> mShaderMacros;
public:
	RenderSystem9();
	virtual ~RenderSystem9();

	bool Initialize(HWND hWnd, RECT vp) override;
	void UpdateFrame(float dt) override;
	void Dispose() override;
	void SetViewPort(int x, int y, int w, int h) override;
	std::string GetPlatform() const override { return "d3d9"; }
public:
	IResourcePtr CreateResource(DeviceResourceType deviceResType) override;

	IFrameBufferPtr LoadFrameBuffer(IResourcePtr res, const Eigen::Vector3i& size, const std::vector<ResourceFormat>& formats) override;
	void SetFrameBuffer(IFrameBufferPtr rendTarget) override;
	void ClearFrameBuffer(IFrameBufferPtr rendTarget, const Eigen::Vector4f& color, float depth, uint8_t stencil) override;

	IIndexBufferPtr LoadIndexBuffer(IResourcePtr res, ResourceFormat format, const Data& data) override;
	void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;

	IVertexBufferPtr LoadVertexBuffer(IResourcePtr res, int stride, int offset, const Data& data) override;
	void SetVertexBuffers(size_t slot, const IVertexBufferPtr vertexBuffers[], size_t count) override;

	IContantBufferPtr LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, HWMemoryUsage usage, const Data& data) override;
	bool UpdateBuffer(IHardwareBufferPtr buffer, const Data& data) override;
	void SetConstBuffers(size_t slot, const IContantBufferPtr buffers[], size_t count, IProgramPtr program) override;

	IBlobDataPtr CompileShader(const ShaderCompileDesc& compileDesc, const Data& data) override;
	IShaderPtr CreateShader(int type, IBlobDataPtr data) override;
	IProgramPtr LoadProgram(IResourcePtr res, const std::vector<IShaderPtr>& shaders) override;
	void SetProgram(IProgramPtr program) override;

	ISamplerStatePtr LoadSampler(IResourcePtr res, const SamplerDesc& samplerDesc) override;
	void SetSamplers(size_t slot, const ISamplerStatePtr samplers[], size_t count) override;

	IInputLayoutPtr LoadLayout(IResourcePtr res, IProgramPtr pProgram, const std::vector<LayoutInputElement>& descArr) override;
	void SetVertexLayout(IInputLayoutPtr layout) override;

	void SetBlendState(const BlendState& blendFunc) override;
	void SetDepthState(const DepthState& depthState) override;

	void SetCullMode(CullMode cullMode) override {}
	void SetFillMode(FillMode fillMode) override {}
	void SetDepthBias(const DepthBias& bias) override {}

	ITexturePtr LoadTexture(IResourcePtr res, ResourceFormat format, 
		const Eigen::Vector4i& w_h_step_arrlen, int mipmap, const Data datas[]) override;
	bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) override;
	void SetTextures(size_t slot, const ITexturePtr textures[], size_t count) override;

	void DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) override;
	void DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) override;

	bool BeginScene() override;
	void EndScene() override;
private:
	IDirect3DVertexDeclaration9* _CreateInputLayout(Program9* pProgram, const std::vector<D3DVERTEXELEMENT9>& descArr);
private:
	bool _GetDeviceCaps();
	void _SetRasterizerState();
	bool _CreateDeviceAndSwapChain();
};

}