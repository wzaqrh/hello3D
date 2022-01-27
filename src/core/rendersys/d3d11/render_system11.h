#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/rendersys/render_system.h"
#include "core/rendersys/d3d11/predeclare.h"

namespace mir {

class RenderSystem11 : public RenderSystem
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	RenderSystem11();
	~RenderSystem11();

	bool Initialize(HWND hWnd, RECT vp) override;
	void Update(float dt) override;
	void Dispose() override;
	void SetViewPort(int x, int y, int w, int h) override;
	std::string GetPlatform() const override { return "d3d11"; }
public:
	IResourcePtr CreateResource(DeviceResourceType deviceResType) override;

	IFrameBufferPtr LoadFrameBuffer(IResourcePtr res, const Eigen::Vector2i& size, const std::vector<ResourceFormat>& formats) override;
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

	ITexturePtr LoadTexture(IResourcePtr res, ResourceFormat format, 
		const Eigen::Vector4i& w_h_step_face, int mipmap, const Data datas[]) override;
	bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) override;
	void SetTextures(size_t slot, const ITexturePtr textures[], size_t count) override;

	void DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) override;
	void DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) override;

	bool BeginScene() override;
	void EndScene() override;
private:
	bool _CreateDeviceAndSwapChain(int width, int height);
	bool _FetchBackFrameBufferColor(int width, int height);
	bool _FetchBackBufferZStencil(int width, int height);
	bool _SetRasterizerState(); 
	bool IsCurrentInMainThread() const;
private:
	HWND mHWnd = NULL;
	D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* mDevice = NULL;
	ID3D11DeviceContext* mDeviceContext = NULL;
	IDXGISwapChain* mSwapChain = NULL;
	
	ID3D11Texture2D* mDepthStencil = NULL;
	ID3D11DepthStencilState* mDepthStencilState = NULL;
	ID3D11BlendState* mBlendState = NULL;

	FrameBuffer11Ptr mBackFrameBuffer, mCurFrameBuffer;

	std::thread::id mMainThreadId;
};

}