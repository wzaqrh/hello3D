#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/if.hpp>
#include "core/mir_config.h"
#include "core/base/debug.h"
#include "core/base/input.h"
#include "core/base/macros.h"
#include "core/rendersys/ogl/render_system_ogl.h"
#include "core/rendersys/ogl/blob_ogl.h"
#include "core/rendersys/ogl/program_ogl.h"
#include "core/rendersys/ogl/input_layout_ogl.h"
#include "core/rendersys/ogl/hardware_buffer_ogl.h"
#include "core/rendersys/ogl/texture_ogl.h"
#include "core/rendersys/ogl/framebuffer_ogl.h"
#include "core/rendersys/ogl/ogl_bind.h"
#include "core/rendersys/ogl/ogl_utils.h"
#include "core/resource/material_factory.h"
#include "core/renderable/renderable.h"

int draw_call_flag = 0;

namespace mir {

void draw_call() {
	if (draw_call_flag) {
		draw_call_flag = draw_call_flag;
	}
	int p = 0;
	++p;
	++p;
}

RenderSystemOGL::RenderSystemOGL()
{}

#if defined MIR_D3D11_DEBUG
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::stringstream ss;
	ss << "---------------" << std::endl;
	ss << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             ss << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ss << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: ss << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     ss << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     ss << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           ss << "Source: Other"; break;
	}
	ss << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               ss << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ss << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         ss << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         ss << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              ss << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          ss << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           ss << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               ss << "Type: Other"; break;
	}
	ss << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         ss << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       ss << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          ss << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: ss << "Severity: notification"; break;
	}
	ss << std::endl;

	OutputDebugStringA(ss.str().c_str());
	MessageBoxA(NULL, ss.str().c_str(), "opengl debug message", MB_OK);
}
#endif
//https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
bool RenderSystemOGL::Initialize(HWND hWnd, RECT vp)
{
	int draw_call_flag__ = draw_call_flag;
	draw_call_flag = 0;

	mMainThreadId = std::this_thread::get_id();
	mHWnd = hWnd;

	{
		HDC hdc = GetDC(mHWnd);
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};
		int index = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, index, &pfd);

		mOglCtx = wglCreateContext(hdc);
		wglMakeCurrent(hdc, mOglCtx);
	}

	if (!gladLoadGL()) {
		MessageBoxA(NULL, "Failed to initialize GLAD", "", MB_OK);
		return false;
	}

	mCaps = std::make_shared<OglCaps>(OglCaps::COMPATIBILITY);
	if (!mCaps->Version.checkVersion(4, 6)) {
		MessageBoxA(NULL, "Require opengl 4.60", "opengl version too low", MB_OK);
		return false;
	}
	mCurVbos.resize(mCaps->Values.MAX_VERTEX_ATTRIB_BINDINGS);

#if defined MIR_D3D11_DEBUG
	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(glDebugOutput, 0);
#endif

	glDisable(GL_PRIMITIVE_RESTART);
	_SetRasterizerState(mCurRasterState = RasterizerState{ kFillSolid, kCullBack });
	_SetDepthState(mCurDepthState = DepthState::Make(kCompareLessEqual, kDepthWriteMaskAll, false));
	_SetBlendState(mCurBlendState = BlendState::MakeAlphaNonPremultiplied());

	if (vp.right == 0 || vp.bottom == 0)
		GetClientRect(mHWnd, &vp);
	mScreenSize.x() = vp.right - vp.left;
	mScreenSize.y() = vp.bottom - vp.top;
	SetViewPort(vp.left, vp.top, mScreenSize.x(), mScreenSize.y());

	SetFrameBuffer(nullptr);

	draw_call_flag = draw_call_flag__;
	return true;
}

RenderSystemOGL::~RenderSystemOGL()
{}
void RenderSystemOGL::Dispose()
{
	wglDeleteContext(mOglCtx);
}

bool RenderSystemOGL::IsCurrentInMainThread() const
{
	return mMainThreadId == std::this_thread::get_id();
}
void RenderSystemOGL::UpdateFrame(float dt)
{}

int RenderSystemOGL::GetGLVersion() const
{
	return mCaps->Version.GetVersion();
}

IResourcePtr RenderSystemOGL::CreateResource(DeviceResourceType deviceResType)
{
	switch (deviceResType) {
	case kDeviceResourceInputLayout:
		return CreateInstance<InputLayoutOGL>();
	case kDeviceResourceProgram:
		return CreateInstance<ProgramOGL>();
	case kDeviceResourceVertexArray: {
		VertexArrayOGLPtr vao = CreateInstance<VertexArrayOGL>();
		GLuint vaoId;
		CheckHR(glGenVertexArrays(1, &vaoId));
		vao->Init(vaoId);
		vao->SetLoaded(true);
		return vao;
	}
	case kDeviceResourceVertexBuffer:
		return CreateInstance<VertexBufferOGL>();
	case kDeviceResourceIndexBuffer:
		return CreateInstance<IndexBufferOGL>();
	case kDeviceResourceContantBuffer:
		return CreateInstance<ContantBufferOGL>();
	case kDeviceResourceTexture:
		return CreateInstance<TextureOGL>();
	case kDeviceResourceFrameBuffer:
		return CreateInstance<FrameBufferOGL>();
	case kDeviceResourceSamplerState:
		return CreateInstance<SamplerStateOGL>();
	default:
		break;
	}
	return nullptr;
}

/********** Framebuffer **********/
IFrameBufferPtr RenderSystemOGL::LoadFrameBuffer(IResourcePtr res, const Eigen::Vector3i& size, const std::vector<ResourceFormat>& formats)
{
	draw_call();
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);
	BOOST_ASSERT(formats.size() >= 1);

	FrameBufferOGLPtr fbo = std::static_pointer_cast<FrameBufferOGL>(res);
	fbo->Init(size.head<2>());
	BindFrameBuffer bindFrambuffer(GL_FRAMEBUFFER, fbo->GetId());

	int colorCount = IF_AND_OR(IsDepthStencil(formats.back()) || formats.back() == kFormatUnknown, formats.size() - 1, formats.size());
	for (size_t i = 0; i < colorCount; ++i)
		fbo->SetAttachColor(i, FrameBufferAttachOGLFactory::CreateColorAttachment(size, formats[i]));
	if (colorCount != formats.size())
		fbo->SetAttachZStencil(FrameBufferAttachOGLFactory::CreateZStencilAttachment(size.head<2>(), formats.back()));

	BOOST_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	return fbo;
}
void RenderSystemOGL::ClearFrameBuffer(IFrameBufferPtr fb, const Eigen::Vector4f& color, float depth, uint8_t stencil)
{
	draw_call();
	BOOST_ASSERT(IsCurrentInMainThread());
	
	if (mCurRasterState.Scissor.ScissorEnable) CheckHR(glDisable(GL_SCISSOR_TEST));

	FrameBufferOGLPtr fbo = std::static_pointer_cast<FrameBufferOGL>(fb);
	if (fbo) {
		GLuint fbId = fbo->GetId();
		CheckHR(glClearNamedFramebufferfv(fbId, GL_COLOR, 0, (GLfloat*)&color));
		if (auto texDS = fbo->GetAttachZStencilTexture()) {
			if (texDS->GetFormat() == kFormatD24UNormS8UInt) {
				CheckHR(glClearNamedFramebufferfi(fbId, GL_DEPTH_STENCIL, 0, depth, stencil));
			}
			else {
				CheckHR(glClearNamedFramebufferfv(fbId, GL_DEPTH, 0, &depth));
			}
		}
	}
	else {
		GLuint fbid = 0;
		CheckHR(glClearNamedFramebufferfv(fbid, GL_COLOR, 0, (GLfloat*)&color));
		CheckHR(glClearNamedFramebufferfi(fbid, GL_DEPTH_STENCIL, 0, depth, stencil));
	}

	if (mCurRasterState.Scissor.ScissorEnable) CheckHR(glEnable(GL_SCISSOR_TEST));
}
void RenderSystemOGL::SetFrameBuffer(IFrameBufferPtr fb)
{
	draw_call();
	BOOST_ASSERT(IsCurrentInMainThread());

	if (mCurFrameBuffer != fb) {
		auto newFbSize = NULLABLE_MEM(fb, GetSize(), mScreenSize);
		auto curFbSize = NULLABLE_MEM(mCurFrameBuffer, GetSize(), mScreenSize);
		if (curFbSize != newFbSize) SetViewPort(0, 0, newFbSize.x(), newFbSize.y());

		mCurFrameBuffer = std::static_pointer_cast<FrameBufferOGL>(fb);
		CheckHR(glBindFramebuffer(GL_FRAMEBUFFER, NULLABLE_MEM(mCurFrameBuffer, GetId(), 0)));
	}
}

/********** Program **********/
IInputLayoutPtr RenderSystemOGL::LoadLayout(IResourcePtr res, IProgramPtr program, const std::vector<LayoutInputElement>& descArr)
{
	draw_call();

	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	InputLayoutOGLPtr layout = std::static_pointer_cast<InputLayoutOGL>(res);
	layout->Init(descArr);
	return layout;
}
void RenderSystemOGL::SetVertexLayout(IInputLayoutPtr res)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(mCurVao != nullptr);
	InputLayoutOGLPtr layout = std::static_pointer_cast<InputLayoutOGL>(res);

	const auto& elements = layout->GetLayoutElements();
	for (size_t slot = 0; slot < elements.size(); ++slot) {
		const LayoutInputElement& elem = elements[slot];
		int location = slot;
		CheckHR(glEnableVertexAttribArray(location));
		auto glFmt = ogl::GetGlFormatInfo(elem.Format);
		CheckHR(glVertexAttribFormat(location, glFmt.ChannelCount, glFmt.InternalType, glFmt.IsNormalized, elem.AlignedByteOffset));
		CheckHR(glVertexAttribBinding(location, elem.InputSlot));
	}
}

IBlobDataPtr RenderSystemOGL::CompileShader(const ShaderCompileDesc& compile, const Data& data)
{
	draw_call();
	//BOOST_ASSERT(IsCurrentInMainThread());

	GLuint shaderId = CheckHR(glCreateShader(ogl::GetGLShaderType((ShaderType)compile.ShaderType)));
	BlobDataOGLPtr blob = CreateInstance<BlobDataOGL>(shaderId);
	blob->mBlob = Data::MakeNull();

	std::string source = "#version 460 core\n";
	{
		if (!compile.Macros.empty()) {
			for (size_t i = 0; i < compile.Macros.size(); ++i) {
				const auto& cdm = compile.Macros[i];
				source += "#define " + cdm.Name + " " + cdm.Definition + "\n";
			}
		}
		source += std::string((const char*)data.Bytes, data.Size);
	}
	const char* sources[] = { source.c_str() };
#if defined MIR_D3D11_DEBUG
	DEBUG_LOG_VERVOSE((boost::format("renderSysOgl.CompileShader type=%d src=\n%s") %compile.ShaderType %source).str());
#endif
	CheckHR(glShaderSource(shaderId, 1, sources, NULL));

	CheckHR(glCompileShader(shaderId));
	if (!ogl::CheckProgramCompileStatus(shaderId)) return nullptr;

	return blob;
}
IShaderPtr RenderSystemOGL::CreateShader(int type, IBlobDataPtr data)
{
	draw_call();

	//BOOST_ASSERT(IsCurrentInMainThread());

	IShaderPtr shader;
	switch (type) {
	case kShaderVertex: {
		shader = CreateInstance<VertexShaderOGL>(data, std::static_pointer_cast<BlobDataOGL>(data)->GetId());
	}break;
	case kShaderPixel: {
		shader = CreateInstance<PixelShaderOGL>(data, std::static_pointer_cast<BlobDataOGL>(data)->GetId());
	}break;
	default:
		break;
	}
	return shader;
}
IProgramPtr RenderSystemOGL::LoadProgram(IResourcePtr res, const std::vector<IShaderPtr>& shaders)
{
	draw_call();

	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	ProgramOGLPtr program = std::static_pointer_cast<ProgramOGL>(res);
	GLuint proId = CheckHR(glCreateProgram());
	program->Init(proId);

	for (auto& iter : shaders) {
		switch (iter->GetType()) {
		case kShaderVertex: {
			auto shader = std::static_pointer_cast<VertexShaderOGL>(iter);
			program->SetVertex(shader);
			CheckHR(glAttachShader(proId, shader->GetId()));
		}break;
		case kShaderPixel: {
			auto shader = std::static_pointer_cast<PixelShaderOGL>(iter);
			program->SetPixel(shader);
			CheckHR(glAttachShader(proId, shader->GetId()));
		}break;
		default:
			break;
		}
	}

	CheckHR(glLinkProgram(proId));
	if (!ogl::CheckProgramLinkStatus(proId)) return nullptr;

	for (auto& iter : shaders) {
		switch (iter->GetType()) {
		case kShaderVertex: {
			auto shader = std::static_pointer_cast<VertexShaderOGL>(iter);
			CheckHR(glDetachShader(proId, shader->GetId()));
		}break;
		case kShaderPixel: {
			auto shader = std::static_pointer_cast<PixelShaderOGL>(iter);
			CheckHR(glDetachShader(proId, shader->GetId()));
		}break;
		default:
			break;
		}
	}

	return program;
}

void RenderSystemOGL::SetProgram(IProgramPtr program)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());

	auto prog = std::static_pointer_cast<ProgramOGL>(program);
	BOOST_ASSERT(ogl::ValidateProgram(prog->GetId()));

	CheckHR(glUseProgram(prog->GetId()));
}

/********** Hardware Buffer **********/
void RenderSystemOGL::SetVertexArray(IVertexArrayPtr vao)
{
	draw_call();

	mCurVao = std::static_pointer_cast<VertexArrayOGL>(vao);
	CheckHR(glBindVertexArray(NULLABLE_MEM(mCurVao, GetId(), 0)));
}

IVertexBufferPtr RenderSystemOGL::LoadVertexBuffer(IResourcePtr res, IVertexArrayPtr ivao, int stride, int offset, const Data& data)
{
	draw_call();

	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res && ivao);

	HWMemoryUsage usage = data.Bytes ? kHWUsageImmutable : kHWUsageDynamic;

	VertexArrayOGLPtr vao = std::static_pointer_cast<VertexArrayOGL>(ivao);
	VertexBufferOGLPtr vbo = std::static_pointer_cast<VertexBufferOGL>(res);
	BindVaoScope bindVao(vao->GetId());
	GLuint vboId = 0;
	{
		CheckHR(glGenBuffers(1, &vboId));
		BindVboScope bindVbo(vboId);
		CheckHR(glBufferStorage(GL_ARRAY_BUFFER, data.Size, data.Bytes, IF_AND_OR(usage == kHWUsageDynamic, GL_MAP_WRITE_BIT, 0)));
	}
	vbo->Init(vao, vboId, data.Size, usage, stride, offset);
	return vbo;
}
void RenderSystemOGL::SetVertexBuffers(size_t slot, const IVertexBufferPtr vertexBuffers[], size_t count)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(count >= 1);

	for (size_t i = 0; i < count; ++i) {
		VertexBufferOGLPtr vbo = std::static_pointer_cast<VertexBufferOGL>(vertexBuffers[i]);
		if (vbo) {
			BOOST_ASSERT(mCurVao == vbo->GetVAO());
			CheckHR(glBindVertexBuffer(slot, vbo->GetId(), vbo->GetOffset(), vbo->GetStride()));
		}
		mCurVbos[slot + i] = vbo;
	}
}

IIndexBufferPtr RenderSystemOGL::LoadIndexBuffer(IResourcePtr res, IVertexArrayPtr ivao, ResourceFormat format, const Data& data)
{
	draw_call();

	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res && ivao);

	HWMemoryUsage usage = data.Bytes ? kHWUsageImmutable : kHWUsageDynamic;

	VertexArrayOGLPtr vao = std::static_pointer_cast<VertexArrayOGL>(ivao);
	IndexBufferOGLPtr vio = std::static_pointer_cast<IndexBufferOGL>(res);
	BindVaoScope bindVao(vao->GetId());
	GLuint vioId = 0;
	{
		CheckHR(glGenBuffers(1, &vioId));
		BindVioScope bindVio(vioId);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vioId);
		CheckHR(glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, data.Size, data.Bytes, IF_AND_OR(usage == kHWUsageDynamic, GL_MAP_WRITE_BIT, 0)));
	}
	vio->Init(std::static_pointer_cast<VertexArrayOGL>(vao), vioId, data.Size, format, usage);
	return vio;
}
void RenderSystemOGL::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());

	if (indexBuffer) {
		IndexBufferOGLPtr vio = std::static_pointer_cast<IndexBufferOGL>(indexBuffer);
		BOOST_ASSERT(mCurVao == vio->GetVAO());
		CheckHR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio->GetId()));
	}
}

IContantBufferPtr RenderSystemOGL::LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, HWMemoryUsage usage, const Data& data)
{
	draw_call();

	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	ContantBufferOGLPtr ubo = std::static_pointer_cast<ContantBufferOGL>(res);
	GLuint uboId = 0;
	CheckHR(glGenBuffers(1, &uboId));
	BindUboScope bindUbo(uboId);
	{
		int byteWidth = (cbDecl.BufferSize + 15) / 16 * 16;
		switch (usage)
		{
		case kHWUsageDefault:
		case kHWUsageImmutable:
			CheckHR(glBufferStorage(GL_UNIFORM_BUFFER, byteWidth, data.Bytes, 0));
			break;
		case kHWUsageDynamic:
			CheckHR(glBufferStorage(GL_UNIFORM_BUFFER, byteWidth, data.Bytes, GL_MAP_WRITE_BIT));
			break;
		case kHWUsageStaging:
		default:
			BOOST_ASSERT(false);
			break;
		}
	}
	ubo->Init(uboId, CreateInstance<ConstBufferDecl>(cbDecl), usage);
	return ubo;
}
void RenderSystemOGL::SetConstBuffers(size_t slot, const IContantBufferPtr buffers[], size_t count, IProgramPtr program)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(count > 0);

	std::vector<GLuint> uboIds(count);
	std::vector<GLintptr> offsets(count);
	std::vector<GLintptr> sizes(count);
	int preIdx = -1;
	for (int i = 0; i < count; ++i) {
		if (buffers[i]) {
			auto ubo = std::static_pointer_cast<ContantBufferOGL>(buffers[i]);
			uboIds[i] = ubo->GetId();
			sizes[i] = ubo->GetBufferSize();
		}
	}
	glBindBuffersRange(GL_UNIFORM_BUFFER, slot, count, &uboIds[0], &offsets[0], &sizes[0]);
	int error = glGetError();
	BOOST_ASSERT(error == 0 || error == GL_INVALID_VALUE);
}

bool RenderSystemOGL::UpdateBuffer(IHardwareBufferPtr buffer, const Data& data)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(buffer);

	if (data.Size == 0) return false;

	switch (buffer->GetType()) {
	case kHWBufferConstant: {
		ContantBufferOGLPtr ubo = std::static_pointer_cast<ContantBufferOGL>(buffer);
		BOOST_ASSERT(ubo->GetUsage() == kHWUsageDynamic);

		BindUboScope bindUbo(ubo->GetId());
		void* mappingData = CheckHR(glMapBufferRange(GL_UNIFORM_BUFFER, 0, data.Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		memcpy(mappingData, data.Bytes, data.Size);
		CheckHR(glUnmapBuffer(GL_UNIFORM_BUFFER));
	}break;
	case kHWBufferVertex: {
		VertexBufferOGLPtr vbo = std::static_pointer_cast<VertexBufferOGL>(buffer);
		BOOST_ASSERT(vbo->GetUsage() == kHWUsageDynamic);

		BindVaoScope bindVao(std::static_pointer_cast<VertexArrayOGL>(vbo->GetVAO())->GetId());
		BindVboScope bindVbo(vbo->GetId());
		void* mappingData = CheckHR(glMapBufferRange(GL_ARRAY_BUFFER, 0, data.Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		memcpy(mappingData, data.Bytes, data.Size);
		CheckHR(glUnmapBuffer(GL_ARRAY_BUFFER));
	}break;
	case kHWBufferIndex: {
		IndexBufferOGLPtr vio = std::static_pointer_cast<IndexBufferOGL>(buffer);
		BOOST_ASSERT(vio->GetUsage() == kHWUsageDynamic);

		BindVaoScope bindVao(std::static_pointer_cast<VertexArrayOGL>(vio->GetVAO())->GetId());
		BindVioScope bindVio(vio->GetId());
		void* mappingData = CheckHR(glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, data.Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		memcpy(mappingData, data.Bytes, data.Size);
		CheckHR(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
	}break;
	default:
		break;
	}
	return true;
}

/********** Texture **********/
ITexturePtr RenderSystemOGL::LoadTexture(IResourcePtr res, ResourceFormat format, const Eigen::Vector4i& size/*w_h_step_face*/, int mipCount, const Data datas[])
{
	draw_call();
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	static Data defaultData = Data::MakeNull();
	datas = datas ? datas : &defaultData;
	const HWMemoryUsage usage = datas[0].Bytes ? kHWUsageDefault : kHWUsageDynamic;

	TextureOGLPtr texture = std::static_pointer_cast<TextureOGL>(res);
	texture->Init(format, usage, size.x(), size.y(), size.w(), mipCount);
	texture->InitTex(datas);
	texture->AutoGenMipmap();
	return texture;
}
void RenderSystemOGL::SetTextures(size_t slot, const ITexturePtr textures[], size_t count)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());

	for (size_t i = 0; i < count; ++i) {
		TextureOGLPtr tex = std::static_pointer_cast<TextureOGL>(textures[i]);
		CheckHR(glBindTextureUnit(slot + i, NULLABLE_MEM(tex, GetId(), 0)));
	}
}
bool RenderSystemOGL::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
{
	return true;
}

ISamplerStatePtr RenderSystemOGL::LoadSampler(IResourcePtr res, const SamplerDesc& desc)
{
	draw_call();

	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	SamplerStateOGLPtr sampler = std::static_pointer_cast<SamplerStateOGL>(res);

	GLuint samId = 0;
	CheckHR(glGenSamplers(1, &samId));
	sampler->Init(samId);

	GLenum minFilter, magFilter, compMode;
	std::tie(minFilter, magFilter, compMode) = ogl::GetGLSamplerFilterMode(desc.Filter);
	CheckHR(glSamplerParameteri(samId, GL_TEXTURE_MIN_FILTER, minFilter));
	CheckHR(glSamplerParameteri(samId, GL_TEXTURE_MAG_FILTER, magFilter));
	CheckHR(glSamplerParameteri(samId, GL_TEXTURE_WRAP_S, ogl::GetGlSamplerAddressMode(desc.AddressU)));
	CheckHR(glSamplerParameteri(samId, GL_TEXTURE_WRAP_T, ogl::GetGlSamplerAddressMode(desc.AddressV)));
	CheckHR(glSamplerParameteri(samId, GL_TEXTURE_WRAP_R, ogl::GetGlSamplerAddressMode(desc.AddressW)));
	float borderColors[4] = { 1,1,1,1 };
	CheckHR(glSamplerParameterfv(samId, GL_TEXTURE_BORDER_COLOR, borderColors));
	CheckHR(glSamplerParameterf(samId, GL_TEXTURE_MIN_LOD, FLT_MIN));
	CheckHR(glSamplerParameterf(samId, GL_TEXTURE_MAX_LOD, FLT_MAX));
	CheckHR(glSamplerParameterf(samId, GL_TEXTURE_LOD_BIAS, 0.0f));
	CheckHR(glSamplerParameteri(samId, GL_TEXTURE_COMPARE_MODE, compMode));
	CheckHR(glSamplerParameteri(samId, GL_TEXTURE_COMPARE_FUNC, ogl::GetGlCompFunc(desc.CmpFunc)));

	return sampler;
}
void RenderSystemOGL::SetSamplers(size_t slot, const ISamplerStatePtr samplers[], size_t count)
{
	draw_call();

	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(samplers && count >= 0);

	for (size_t i = 0; i < count; ++i) {
		SamplerStateOGLPtr sampler = std::static_pointer_cast<SamplerStateOGL>(samplers[i]);
		CheckHR(glBindSampler(slot + i, NULLABLE_MEM(sampler, GetId(), 0)));
	}
}

/********** State **********/
void RenderSystemOGL::_SetViewPort(const Eigen::Vector4i& newVp)
{
	draw_call();

	CheckHR(glDepthRange(0.0f, 1.0f));
	CheckHR(glViewport(newVp[0], newVp[1], newVp[2], newVp[3]));
}
void RenderSystemOGL::SetViewPort(int x, int y, int width, int height)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetViewPort");
	//BOOST_ASSERT(IsCurrentInMainThread());

	Eigen::Vector4i newVP(x, y, width, height);
	if (mCurViewPort != newVP) {
		_SetViewPort(newVP);
		mCurViewPort = newVP;
	}
}

void RenderSystemOGL::_SetBlendState(const BlendState& blendFunc)
{
	draw_call();

	if ((blendFunc.Src != kBlendOne) || (blendFunc.Dst != kBlendZero)) { CheckHR(glEnable(GL_BLEND)); }
	else { CheckHR(glDisable(GL_BLEND)); }

	CheckHR(glBlendEquation(GL_FUNC_ADD));
	CheckHR(glBlendFunc(ogl::GetGlBlendFunc(blendFunc.Src), ogl::GetGlBlendFunc(blendFunc.Dst)));
}
void RenderSystemOGL::SetBlendState(const BlendState& blendFunc)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetBlendState");
	BOOST_ASSERT(IsCurrentInMainThread());

	if (mCurBlendState != blendFunc) {
		_SetBlendState(blendFunc);
		mCurBlendState = blendFunc;
	}
}

void RenderSystemOGL::_SetDepthState(const DepthState& depthState)
{
	draw_call();

	if (depthState.DepthEnable) {
		CheckHR(glEnable(GL_DEPTH_TEST));
		CheckHR(glDepthMask(depthState.WriteMask));
		CheckHR(glDepthFunc(ogl::GetGlCompFunc(depthState.CmpFunc)));
	}
	else {
		CheckHR(glDisable(GL_DEPTH_TEST));
	}
	CheckHR(glDisable(GL_STENCIL_TEST));
}
void RenderSystemOGL::SetDepthState(const DepthState& depthState)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetDepthState");
	BOOST_ASSERT(IsCurrentInMainThread());

	if (mCurDepthState != depthState) {
		_SetDepthState(depthState);
		mCurDepthState = depthState;
	}
}

void RenderSystemOGL::_SetCullMode(CullMode cullMode)
{
	draw_call();

	switch (cullMode) {
	case kCullBack: CheckHR(glCullFace(GL_BACK)); if (mCurRasterState.CullMode == kCullNone) CheckHR(glEnable(GL_CULL_FACE)); break;
	case kCullFront: CheckHR(glCullFace(GL_FRONT)); if (mCurRasterState.CullMode == kCullNone) CheckHR(glEnable(GL_CULL_FACE)); break;
	case kCullNone: CheckHR(glDisable(GL_CULL_FACE)); break;
	default: BOOST_ASSERT(FALSE); break;
	}
}
void RenderSystemOGL::SetCullMode(CullMode cullMode)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetCullMode");
	BOOST_ASSERT(IsCurrentInMainThread());

	if (mCurRasterState.CullMode != cullMode) {
		_SetCullMode(cullMode);
		mCurRasterState.CullMode = cullMode;
	}
}

void RenderSystemOGL::_SetFillMode(FillMode fillMode)
{
	draw_call();

	switch (fillMode) {
	case kFillWireFrame: CheckHR(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)); break;
	case kFillSolid: CheckHR(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)); break;
	default: BOOST_ASSERT(FALSE); break;
	}
}
void RenderSystemOGL::SetFillMode(FillMode fillMode)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetFillMode");
	BOOST_ASSERT(IsCurrentInMainThread());

	if (mCurRasterState.FillMode != fillMode) {
		_SetFillMode(fillMode);
		mCurRasterState.FillMode = fillMode;
	}
}

void RenderSystemOGL::_SetDepthBias(const DepthBias& bias)
{
	//todo
}
void RenderSystemOGL::SetDepthBias(const DepthBias& bias)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetDepthBias");
	BOOST_ASSERT(IsCurrentInMainThread());

	if (mCurRasterState.DepthBias != bias) {
		_SetDepthBias(bias);
		mCurRasterState.DepthBias = bias;
	}
}

void RenderSystemOGL::_SetScissorState(const ScissorState& scissor)
{
	draw_call();

	if (mCurRasterState.Scissor.ScissorEnable != scissor.ScissorEnable) {
		if (scissor.ScissorEnable) { CheckHR(glEnable(GL_SCISSOR_TEST)); }
		else { CheckHR(glDisable(GL_SCISSOR_TEST)); }
	}

	const auto& rct = scissor.Rect;
	CheckHR(glScissor(rct[0], mCurViewPort[3] - rct[3], rct[2] - rct[0], rct[3] - rct[1]));
	CheckHR(glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE));
}
void RenderSystemOGL::SetScissorState(const ScissorState& scissor)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetScissorState");
	BOOST_ASSERT(IsCurrentInMainThread());

	if (mCurRasterState.Scissor != scissor) {
		_SetScissorState(scissor);
		mCurRasterState.Scissor = scissor;
	}
}

void RenderSystemOGL::_SetRasterizerState(const RasterizerState& rs)
{
	draw_call();

	_SetCullMode(rs.CullMode);
	_SetFillMode(rs.FillMode);
	_SetDepthBias(rs.DepthBias);
	_SetScissorState(rs.Scissor);
}

/********** Draw **********/
void RenderSystemOGL::DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	draw_call();
	BOOST_ASSERT(IsCurrentInMainThread());

	CheckHR(glDrawArrays(ogl::GetGLTopologyType(topo), 0, op.VertexBuffers[0]->GetCount()));
}
void RenderSystemOGL::DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	draw_call();
	BOOST_ASSERT(IsCurrentInMainThread());

	int indexCount = IF_OR(op.IndexCount, op.IndexBuffer->GetBufferSize() / op.IndexBuffer->GetWidth());
	auto glFmt = ogl::GetGlFormatInfo(op.IndexBuffer->GetFormat());
	CheckHR(glDrawElementsBaseVertex(ogl::GetGLTopologyType(topo), indexCount, glFmt.InternalType, (void*)(op.IndexPos * op.IndexBuffer->GetWidth()), op.IndexBase));
}

bool RenderSystemOGL::BeginScene()
{
	return true;
}
void RenderSystemOGL::EndScene(BOOL vsync)
{
	draw_call();

	HDC hdc = GetDC(mHWnd);
	SwapBuffers(hdc);
}

}