#pragma once
#include "core/rendersys/render_system.h"
#include "interface_type11.h"

namespace mir {

class RenderSystem11 : public RenderSystem
{
private:
	HWND mHWnd = NULL;
	D3D_DRIVER_TYPE mDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* mDevice = NULL;
	ID3D11DeviceContext* mDeviceContext = NULL;
	IDXGISwapChain* mSwapChain = NULL;
	ID3D11Texture2D* mDepthStencil = NULL;
	ID3D11DepthStencilState* mDepthStencilState = NULL;
	ID3D11BlendState* mBlendState = NULL;

	ID3D11RenderTargetView* mBackRenderTargetView = NULL;
	ID3D11DepthStencilView* mBackDepthStencilView = NULL;
	ID3D11RenderTargetView* mCurRenderTargetView = NULL;
	ID3D11DepthStencilView* mCurDepthStencilView = NULL;

	ThreadPumpPtr mThreadPump;
	std::vector<D3D_SHADER_MACRO> mShaderMacros;
public:
	RenderSystem11();
	~RenderSystem11();

	bool Initialize(HWND hWnd, RECT vp) override;
	void Update(float dt) override;
	void CleanUp() override;
	void SetViewPort(int x, int y, int w, int h) override;
public:
	void ClearColorDepthStencil(const Eigen::Vector4f& color, float depth, unsigned char stencil) override;

	IResourcePtr CreateResource(DeviceResourceType deviceResType) override;

	IRenderTexturePtr LoadRenderTexture(IResourcePtr res, int width, int height, ResourceFormat format) override;
	void _ClearRenderTexture(IRenderTexturePtr rendTarget, const Eigen::Vector4f& color, float depth, unsigned char stencil);
	void SetRenderTarget(IRenderTexturePtr rendTarget) override;

	IIndexBufferPtr LoadIndexBuffer(IResourcePtr res, int bufferSize, ResourceFormat format, void* buffer) override;
	void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;

	IVertexBufferPtr LoadVertexBuffer(IResourcePtr res, int bufferSize, int stride, int offset, void* buffer) override;
	void SetVertexBuffer(IVertexBufferPtr vertexBuffer) override;

	IContantBufferPtr LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, void* data) override;
	bool UpdateBuffer(IHardwareBufferPtr buffer, void* data, int dataSize) override;
	void UpdateConstBuffer(IContantBufferPtr buffer, void* data, int dataSize) override;
	void SetConstBuffers(size_t slot, IContantBufferPtr buffers[], size_t count, IProgramPtr program) override;

	IProgramPtr CreateProgramByCompile(IResourcePtr res, const std::string& vsPath, 
		const std::string& psPath, 
		const std::string& vsEntry, 
		const std::string& psEntry) override;
	IProgramPtr CreateProgramByFXC(IResourcePtr res, const std::string& name, 
		const std::string& vsEntry, 
		const std::string& psEntry) override;
	void SetProgram(IProgramPtr program) override;

	ISamplerStatePtr LoadSampler(IResourcePtr res, SamplerFilterMode filter, CompareFunc comp) override;
	void SetSamplers(size_t slot, ISamplerStatePtr samplers[], size_t count) override;

	IInputLayoutPtr LoadLayout(IResourcePtr res, IProgramPtr pProgram, const LayoutInputElement descArray[], size_t descCount) override;
	void SetVertexLayout(IInputLayoutPtr layout) override;

	void SetBlendFunc(const BlendState& blendFunc) override;
	void SetDepthState(const DepthState& depthState) override;

	ITexturePtr CreateTexture(int width, int height, ResourceFormat format, int mipmap) override;
	bool LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep) override;
	void SetTexture(size_t slot, ITexturePtr texture) override;
	void SetTextures(size_t slot, ITexturePtr textures[], size_t count) override;

	void DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) override;
	void DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) override;

	bool BeginScene() override;
	void EndScene() override;
protected:
	virtual ITexturePtr _CreateTexture(IResourcePtr res, const char* pSrcFile, 
		ResourceFormat format, bool async, bool isCube);
private:
	HRESULT _CreateDeviceAndSwapChain(int width, int height);
	HRESULT _CreateBackRenderTargetView();
	HRESULT _CreateBackDepthStencilView(int width, int height);
	void _SetViewports(int width, int height, int x = 0, int y = 0);
	HRESULT _SetRasterizerState(); 
private:
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize, void* buffer);
	ID3D11Buffer* _CreateVertexBuffer(int bufferSize);

	VertexShader11Ptr _CreateVS(const std::string& filename, const std::string& entry, bool async = true);
	VertexShader11Ptr _CreateVSByFXC(const std::string& filename);
	PixelShader11Ptr _CreatePS(const std::string& filename, const std::string& entry, bool async = true);
	PixelShader11Ptr _CreatePSByFXC(const std::string& filename);

	ID3D11InputLayout* _CreateInputLayout(Program11* pProgram, const std::vector<D3D11_INPUT_ELEMENT_DESC>& descArr);
};

}