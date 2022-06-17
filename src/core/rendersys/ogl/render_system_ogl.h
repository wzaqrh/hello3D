#pragma once
#include <windows.h>
#include <glad/glad.h>
#include <GL/GL.h>
#include "core/rendersys/render_system.h"
#include "core/rendersys/ogl/predeclare.h"
#include "core/rendersys/ogl/ogl_caps.h"

namespace mir {

class RenderSystemOGL : public RenderSystem
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	RenderSystemOGL();
	~RenderSystemOGL();

	bool Initialize(HWND hWnd, RECT vp) override;
	void UpdateFrame(float dt) override;
	void Dispose() override;
	void SetViewPort(int x, int y, int w, int h) override;
	std::string GetPlatform() const override { return "ogl"; }
public:
	IResourcePtr CreateResource(DeviceResourceType deviceResType) override;

	IFrameBufferPtr LoadFrameBuffer(IResourcePtr res, const Eigen::Vector3i& size, const std::vector<ResourceFormat>& formats) override;
	void SetFrameBuffer(IFrameBufferPtr rendTarget) override;
	void ClearFrameBuffer(IFrameBufferPtr rendTarget, const Eigen::Vector4f& color, float depth, uint8_t stencil) override;

	void SetVertexArray(IVertexArrayPtr vao) override;

	IIndexBufferPtr LoadIndexBuffer(IResourcePtr res, IVertexArrayPtr vao, ResourceFormat format, const Data& data) override;
	void SetIndexBuffer(IIndexBufferPtr indexBuffer) override;

	IVertexBufferPtr LoadVertexBuffer(IResourcePtr res, IVertexArrayPtr vao, int stride, int offset, const Data& data) override;
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
	void EndScene(BOOL vsync) override;
private:
	bool IsCurrentInMainThread() const;
private:
	OglCaps mCaps;
	HWND mHWnd = NULL;
	FrameBufferOGLPtr mBackFrameBuffer, mCurFrameBuffer;
	std::thread::id mMainThreadId;
private:
	std::vector<VertexBufferOGLPtr> mCurrentVbos;
	VertexArrayOGLPtr mCurrentVao;
};

}