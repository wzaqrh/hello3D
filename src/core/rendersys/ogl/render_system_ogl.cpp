#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/if.hpp>
#include <wrl/client.h>
#include "core/mir_config.h"
#include "core/base/d3d.h"
#include "core/base/debug.h"
#include "core/base/input.h"
#include "core/base/macros.h"
#include "core/rendersys/ogl/ogl_utils.h"
#include "core/rendersys/ogl/render_system_ogl.h"
#include "core/rendersys/ogl/blob_ogl.h"
#include "core/rendersys/ogl/program_ogl.h"
#include "core/rendersys/ogl/input_layout_ogl.h"
#include "core/rendersys/ogl/hardware_buffer_ogl.h"
#include "core/rendersys/ogl/texture_ogl.h"
#include "core/rendersys/ogl/framebuffer_ogl.h"
#include "core/resource/material_factory.h"
#include "core/renderable/renderable.h"

namespace mir {

RenderSystemOGL::RenderSystemOGL()
: mCaps(OglCaps::COMPATIBILITY)
{}
//https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
bool RenderSystemOGL::Initialize(HWND hWnd, RECT vp)
{
	mMainThreadId = std::this_thread::get_id();
	mHWnd = hWnd;

	{
		HDC hdc = GetDC(hWnd);
		PIXELFORMATDESCRIPTOR pfd =
		{
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

		HGLRC ourOpenGLRenderingContext = wglCreateContext(hdc);
		wglMakeCurrent(hdc, ourOpenGLRenderingContext);
		//MessageBoxA(0, (char*)glGetString(GL_VERSION), "OPENGL VERSION", 0);
		wglDeleteContext(ourOpenGLRenderingContext);
	}

	{
		GLint maxVertAttrbBinding = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &maxVertAttrbBinding);
		mCurrentVbos.resize(maxVertAttrbBinding);
	}

	SetFrameBuffer(nullptr);
	SetDepthState(DepthState::Make(kCompareLessEqual, kDepthWriteMaskAll, true));
	SetBlendState(BlendState::MakeAlphaPremultiplied());

	if (vp.right == 0 || vp.bottom == 0) 
		GetClientRect(mHWnd, &vp);
	mScreenSize.x() = vp.right - vp.left;
	mScreenSize.y() = vp.bottom - vp.top;
	SetViewPort(vp.left, vp.top, mScreenSize.x(), mScreenSize.y());
	return true;
}

RenderSystemOGL::~RenderSystemOGL()
{}
void RenderSystemOGL::Dispose()
{}

bool RenderSystemOGL::IsCurrentInMainThread() const
{
	return mMainThreadId == std::this_thread::get_id();
}

void RenderSystemOGL::UpdateFrame(float dt)
{}

void RenderSystemOGL::SetViewPort(int x, int y, int width, int height)
{
	glDepthRange(0.0f, 1.0f);
	glViewport(x, y, width, height);
}

IResourcePtr RenderSystemOGL::CreateResource(DeviceResourceType deviceResType)
{
	switch (deviceResType) {
	case kDeviceResourceInputLayout:
		return CreateInstance<InputLayoutOGL>();
	case kDeviceResourceProgram:
		return CreateInstance<ProgramOGL>();
	case kDeviceResourceVertexArray: {
		GLuint vaoId;
		glGenVertexArrays(1, &vaoId);
		VertexArrayOGLPtr vao = CreateInstance<VertexArrayOGL>(vaoId);
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

/********** LoadFrameBuffer **********/
static TextureOGLPtr _CreateColorAttachTexture(const Eigen::Vector2i& size, ResourceFormat format)
{
	constexpr bool autoGen = false;
	constexpr size_t mipCount = 1;

	TextureOGLPtr texture = CreateInstance<TextureOGL>();
	GLuint texId = 0;
	glGenTextures(1, &texId);
	texture->Init(texId, format, kHWUsageDefault, size.x(), size.y(), 1, mipCount);

	constexpr GLenum glTarget = GL_TEXTURE_2D;
	glBindTexture(glTarget, texId);
	{
		glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, mipCount);

		glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(glTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x(), size.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	glBindTexture(glTarget, 0);

	texture->SetLoaded();
	return texture;
}
static TextureOGLPtr _CreateZStencilAttachTexture(const Eigen::Vector2i& size, ResourceFormat format)
{
	BOOST_ASSERT(d3d::IsDepthStencil(static_cast<DXGI_FORMAT>(format)));
	BOOST_ASSERT(format == kFormatD24UNormS8UInt 
		|| format == kFormatD32Float
		|| format == kFormatD16UNorm);

	constexpr bool autoGen = false;
	constexpr size_t mipCount = 1;
	
	const DXGI_FORMAT dsvFmt = static_cast<DXGI_FORMAT>(format);
	const DXGI_FORMAT texFmt = (format == kFormatD24UNormS8UInt) ? DXGI_FORMAT_R24G8_TYPELESS : dsvFmt;
	const DXGI_FORMAT srvFmt = (format == kFormatD24UNormS8UInt) ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : dsvFmt;

	TextureOGLPtr texture = CreateInstance<TextureOGL>();
	GLuint texId = 0;
	glGenTextures(1, &texId);
	texture->Init(texId, format, kHWUsageDefault, size.x(), size.y(), 1, mipCount);

	constexpr GLenum glTarget = GL_TEXTURE_2D;
	glBindTexture(glTarget, texId);
	{
		glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, mipCount);

		glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(glTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x(), size.y(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	glBindTexture(glTarget, 0);

	texture->SetLoaded();
	return texture;
}
static FrameBufferAttachOGLPtr _CreateFrameBufferAttachColor(const Eigen::Vector2i& size, ResourceFormat format)
{
	auto texture = _CreateColorAttachTexture(size, format);
	return format == kFormatUnknown ? nullptr : CreateInstance<FrameBufferAttachOGL>(texture);
}
static FrameBufferAttachOGLPtr _CreateFrameBufferAttachZStencil(const Eigen::Vector2i& size, ResourceFormat format) 
{
	auto texture = _CreateZStencilAttachTexture(size, format);
	return format == kFormatUnknown ? nullptr : CreateInstance<FrameBufferAttachOGL>(texture);
}
IFrameBufferPtr RenderSystemOGL::LoadFrameBuffer(IResourcePtr res, const Eigen::Vector3i& size, const std::vector<ResourceFormat>& formats)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);
	BOOST_ASSERT(formats.size() >= 1);
	BOOST_ASSERT(d3d::IsDepthStencil(static_cast<DXGI_FORMAT>(formats.back())) || formats.back() == kFormatUnknown);

	GLuint fbId = 0;
	glGenFramebuffers(1, &fbId);
	BindFrameBuffer bindFrambuffer(GL_FRAMEBUFFER, fbId);

	FrameBufferOGLPtr framebuffer = std::static_pointer_cast<FrameBufferOGL>(res);
	framebuffer->Init(fbId, size);
	for (size_t i = 0; i + 1 < formats.size(); ++i) {
		auto attachI = _CreateFrameBufferAttachColor(size, formats[i]);
		framebuffer->SetAttachColor(i, attachI);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, std::static_pointer_cast<TextureOGL>(attachI->AsTexture())->GetId(), 0);
	}
	auto attachZS = _CreateFrameBufferAttachZStencil(size, formats.size() >= 2 ? formats.back() : kFormatUnknown);
	framebuffer->SetAttachZStencil(attachZS);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, std::static_pointer_cast<TextureOGL>(attachZS->AsTexture())->GetId(), 0);

	return framebuffer;
}
void RenderSystemOGL::ClearFrameBuffer(IFrameBufferPtr fb, const Eigen::Vector4f& color, float depth, uint8_t stencil)
{
	BOOST_ASSERT(IsCurrentInMainThread());
	FrameBufferOGLPtr fbo = std::static_pointer_cast<FrameBufferOGL>(fb);
#if 1
	glClearNamedFramebufferfv(fbo->GetId(), GL_COLOR, 0, (GLfloat*)&color);
	glClearNamedFramebufferfi(fbo->GetId(), GL_DEPTH, 0, depth, stencil);
#elif 0
	BindFrameBuffer bindFrambuffer(GL_FRAMEBUFFER, fbo->GetId());
	glClearBufferfv(GL_DEPTH, 0, static_cast<const GLfloat*>(&depth));
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
#else
	BindFrameBuffer bindFrambuffer(GL_FRAMEBUFFER, fbo->GetId());
	glClearColor(color.x(), color.y(), color.z(), color.w());
	glClearDepth(depth);
	glClearStencil(stencil);
#endif
}
void RenderSystemOGL::SetFrameBuffer(IFrameBufferPtr fb)
{
	BOOST_ASSERT(IsCurrentInMainThread());

	auto prevFbSize = mCurFrameBuffer->GetSize();

	auto fbo = std::static_pointer_cast<FrameBufferOGL>(fb);
	mCurFrameBuffer = fbo ? fbo : mBackFrameBuffer;

	if (mCurFrameBuffer->GetSize() != prevFbSize) {
		auto fbsize = mCurFrameBuffer->GetSize();
		SetViewPort(0, 0, fbsize.x(), fbsize.y());
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mCurFrameBuffer->GetId());
}

IInputLayoutPtr RenderSystemOGL::LoadLayout(IResourcePtr res, IProgramPtr pProgram, const std::vector<LayoutInputElement>& descArr)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res && AsRes(pProgram)->IsLoaded());

	InputLayoutOGLPtr layout = std::static_pointer_cast<InputLayoutOGL>(res);
	layout->Init(descArr);
	return layout;
}
void RenderSystemOGL::SetVertexLayout(IInputLayoutPtr res) 
{
	BOOST_ASSERT(IsCurrentInMainThread());
	InputLayoutOGLPtr layout = std::static_pointer_cast<InputLayoutOGL>(res);

	const auto& elements = layout->GetLayoutElements();
	for (size_t slot = 0; slot < elements.size(); ++slot) {
		const auto& elem = elements[slot];
		glEnableVertexAttribArray(elem.Location);
		GLenum glFmt = ogl::GetGLFormat(elem.Format);
		size_t glFmtChannels = ogl::GetChannelCount(elem.Format);
		bool normalized = ogl::IsNormalized(elem.Format);
		glVertexAttribFormat(elem.Location, glFmtChannels, glFmt, normalized, elem.AlignedByteOffset);
		glVertexAttribBinding(elem.Location, elem.InputSlot);
	}
}

IBlobDataPtr RenderSystemOGL::CompileShader(const ShaderCompileDesc& compile, const Data& data)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	GLuint sbo = glCreateShader(ogl::GetShaderType((ShaderType)type));
	BlobDataOGLPtr blob = CreateInstance<BlobDataOGL>(sbo);

	std::string source = "#version 450 core\n";
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
	glShaderSource(sbo, 1, sources, NULL);

	glCompileShader(sbo);
	{
		GLint compileState = GL_FALSE;
		glGetShaderiv(sbo, GL_COMPILE_STATUS, &compileState);
		if (!compileState) {
			int logLength;
			glGetShaderiv(sbo, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<char> errorLog(logLength);
			glGetShaderInfoLog(sbo, logLength, NULL, &errorLog[0]);
			DEBUG_LOG_ERROR((boost::format("%s\n") % &errorLog[0]).str().c_str());
			
			blob = nullptr;
		}
	}

	return blob;
}
IShaderPtr RenderSystemOGL::CreateShader(int type, IBlobDataPtr data)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	switch (type) {
	case kShaderVertex: {
		VertexShaderOGLPtr ret = CreateInstance<VertexShaderOGL>(data, std::static_pointer_cast<BlobDataOGL>(data)->GetId());	
		return ret;
	}break;
	case kShaderPixel: {
		PixelShaderOGLPtr ret = CreateInstance<PixelShaderOGL>(data, std::static_pointer_cast<BlobDataOGL>(data)->GetId());
		return ret;
	}break;
	default:
		break;
	}
	return nullptr;
}
IProgramPtr RenderSystemOGL::LoadProgram(IResourcePtr res, const std::vector<IShaderPtr>& shaders)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	ProgramOGLPtr program = std::static_pointer_cast<ProgramOGL>(res);
	GLuint proId = glCreateProgram();
	program->Init(proId);

	for (auto& iter : shaders) {
		switch (iter->GetType()) {
		case kShaderVertex:{
			auto shader = std::static_pointer_cast<VertexShaderOGL>(iter);
			program->SetVertex(shader);
			glAttachShader(proId, shader->GetId());
		}break;
		case kShaderPixel: {
			auto shader = std::static_pointer_cast<PixelShaderOGL>(iter);
			program->SetPixel(shader);
			glAttachShader(proId, shader->GetId());
		}break;
		default:
			break;
		}
	}

	for (size_t slot = 0; slot < mCaps.Limits.MAX_UNIFORM_BUFFER_BINDINGS; ++slot)
		glUniformBlockBinding(proId, slot, slot);

	glLinkProgram(proId);
	{
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(proId, GL_LINK_STATUS, &linkStatus);
		if (!linkStatus) {
			int logLength;
			glGetProgramiv(proId, GL_INFO_LOG_LENGTH, &logLength);

			std::vector<char> errorLog(logLength);
			glGetProgramInfoLog(proId, logLength, NULL, &errorLog[0]);
			DEBUG_LOG_ERROR((boost::format("%s\n") % &errorLog[0]).str().c_str());

			program = nullptr;
		}
	}

	return program;
}

bool _ValidateProgram(GLuint proId)
{
	if (proId == 0)
		return false;

	glValidateProgram(proId);
	GLint validateRes = GL_FALSE;
	glGetProgramiv(proId, GL_VALIDATE_STATUS, &validateRes);

	if (!validateRes) {
		int logLength;
		glGetProgramiv(proId, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<char> errorLog(logLength);
		glGetProgramInfoLog(proId, logLength, NULL, &errorLog[0]);
		DEBUG_LOG_ERROR((boost::format("%s\n") % &errorLog[0]).str().c_str());
	}
	return validateRes == GL_TRUE;
}

void RenderSystemOGL::SetProgram(IProgramPtr program)
{
	BOOST_ASSERT(IsCurrentInMainThread());

	auto prog = std::static_pointer_cast<ProgramOGL>(program);
	glUseProgram(prog->GetId());
}

void RenderSystemOGL::SetVertexArray(IVertexArrayPtr vao) 
{	
	mCurrentVao = std::static_pointer_cast<VertexArrayOGL>(vao);
	glBindBuffer(GL_ARRAY_BUFFER, IF_AND_OR(mCurrentVao, mCurrentVao->GetId(), 0));
}

IVertexBufferPtr RenderSystemOGL::LoadVertexBuffer(IResourcePtr res, IVertexArrayPtr ivao, int stride, int offset, const Data& data)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res && ivao);

	HWMemoryUsage usage = data.Bytes ? kHWUsageImmutable : kHWUsageDynamic;

	VertexArrayOGLPtr vao = std::static_pointer_cast<VertexArrayOGL>(ivao);
	VertexBufferOGLPtr vbo = std::static_pointer_cast<VertexBufferOGL>(res);
	BindVaoScope bindVao(vao->GetId());
	GLuint vboId = 0;
	{
		glGenBuffers(1, &vboId);
		BindVboScope bindVbo(vboId);
		glBufferStorage(GL_ARRAY_BUFFER, data.Size, data.Bytes, IF_AND_OR(usage == kHWUsageDynamic, GL_MAP_WRITE_BIT, 0));
	}
	vbo->Init(vao, vboId, data.Size, usage, stride, offset);
	return vbo;
}
void RenderSystemOGL::SetVertexBuffers(size_t slot, const IVertexBufferPtr vertexBuffers[], size_t count)
{
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(count >= 1);

	for (size_t i = 0; i < count; ++i) {
		VertexBufferOGLPtr vbo = std::static_pointer_cast<VertexBufferOGL>(vertexBuffers[i]);
		if (vbo) {
			glBindVertexBuffer(slot, vbo->GetId(), vbo->GetOffset(), vbo->GetStride());
			BOOST_ASSERT(mCurrentVao == vbo->GetVAO());
		}
		else glBindVertexBuffer(slot, 0, 0, 0);
		mCurrentVbos[slot + i] = std::static_pointer_cast<VertexBufferOGL>(vertexBuffers[i]);
	}
}

IIndexBufferPtr RenderSystemOGL::LoadIndexBuffer(IResourcePtr res, IVertexArrayPtr ivao, ResourceFormat format, const Data& data)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res && ivao);

	HWMemoryUsage usage = data.Bytes ? kHWUsageImmutable : kHWUsageDynamic;

	VertexArrayOGLPtr vao = std::static_pointer_cast<VertexArrayOGL>(ivao);
	IndexBufferOGLPtr vio = std::static_pointer_cast<IndexBufferOGL>(res);
	BindVaoScope bindVao(vao->GetId()); 
	GLuint vioId = 0;
	{
		glGenBuffers(1, &vioId);
		BindVioScope bindVio(vioId);
		glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, data.Size, data.Bytes, IF_AND_OR(usage == kHWUsageDynamic, GL_MAP_WRITE_BIT, 0));
	}
	vio->Init(std::static_pointer_cast<VertexArrayOGL>(vao), vioId, data.Size, format, usage);
	return vio;
}
void RenderSystemOGL::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	BOOST_ASSERT(IsCurrentInMainThread());

	if (indexBuffer) {
		IndexBufferOGLPtr vio = std::static_pointer_cast<IndexBufferOGL>(indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio->GetId());
		BOOST_ASSERT(mCurrentVao == vio->GetVAO());
	}
}

IContantBufferPtr RenderSystemOGL::LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, HWMemoryUsage usage, const Data& data)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);
	BOOST_ASSERT((cbDecl.BufferSize & 15) == 0);

	ContantBufferOGLPtr ubo = std::static_pointer_cast<ContantBufferOGL>(res);
	GLuint uboId = 0;
	glGenBuffers(1, &uboId);
	BindUboScope bindUbo(uboId); {
		switch (usage)
		{
		case kHWUsageDefault:
		case kHWUsageImmutable:
			glBufferStorage(GL_UNIFORM_BUFFER, cbDecl.BufferSize, data.Bytes, 0);
			break;
		case kHWUsageDynamic:
			glBufferStorage(GL_UNIFORM_BUFFER, cbDecl.BufferSize, data.Bytes, GL_MAP_WRITE_BIT);
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
	BOOST_ASSERT(IsCurrentInMainThread());

	std::vector<GLuint> uboIds(count);
	for (size_t i = 0; i < count; ++i)
		uboIds[i] = buffers[i] ? std::static_pointer_cast<ContantBufferOGL>(buffers[i])->GetId() : 0;
	glBindBuffersRange(GL_UNIFORM_BUFFER, slot, count, &uboIds[0], nullptr, nullptr);
	//glUniformBlockBinding( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
}

bool RenderSystemOGL::UpdateBuffer(IHardwareBufferPtr buffer, const Data& data)
{
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(buffer);

	switch (buffer->GetType()) {
	case kHWBufferConstant: {
		ContantBufferOGLPtr ubo = std::static_pointer_cast<ContantBufferOGL>(buffer);
		BOOST_ASSERT(ubo->GetUsage() == kHWUsageDynamic);

		glBindBuffer(GL_UNIFORM_BUFFER, ubo->GetId());
		void* mappingData = glMapBufferRange(GL_UNIFORM_BUFFER, 0, data.Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		memcpy(mappingData, data.Bytes, data.Size);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}break;
	case kHWBufferVertex: {
		VertexBufferOGLPtr vbo = std::static_pointer_cast<VertexBufferOGL>(buffer);
		BOOST_ASSERT(vbo->GetUsage() == kHWUsageDynamic);

		glBindBuffer(GL_ARRAY_BUFFER, vbo->GetId());
		void* mappingData = glMapBufferRange(GL_ARRAY_BUFFER, 0, data.Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		memcpy(mappingData, data.Bytes, data.Size);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}break;
	case kHWBufferIndex: {
		IndexBufferOGLPtr ibo = std::static_pointer_cast<IndexBufferOGL>(buffer);
		BOOST_ASSERT(ibo->GetUsage() == kHWUsageDynamic);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->GetId());
		void* mappingData = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, data.Size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		memcpy(mappingData, data.Bytes, data.Size);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}break;
	default:
		break;
	}
	return true;
}

ITexturePtr RenderSystemOGL::LoadTexture(IResourcePtr res, ResourceFormat format, const Eigen::Vector4i& size/*w_h_step_face*/, int mipCount, const Data datas[])
{
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	GLuint texId = 0;
	glGenTextures(1, &texId);

	Data defaultData = Data{};
	datas = datas ? datas : &defaultData;
	const HWMemoryUsage usage = datas[0].Bytes ? kHWUsageDefault : kHWUsageDynamic;

	TextureOGLPtr texture = std::static_pointer_cast<TextureOGL>(res);
	texture->Init(texId, format, usage, size.x(), size.y(), size.w(), mipCount);

	mipCount = texture->GetMipmapCount();
	const bool autoGen = texture->IsAutoGenMipmap();
	const size_t faceCount = texture->GetFaceCount();
	constexpr int imageSize = 0;//only used for 3d textures
	BOOST_ASSERT_IF_THEN(faceCount > 1, datas[0].Bytes);
	BOOST_ASSERT_IF_THEN(autoGen, datas[0].Bytes && faceCount == 1);

	GLenum glTarget = IF_AND_OR(faceCount > 1, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D);
	glBindTexture(glTarget, texId);
	glTexParameteri(glTarget, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(glTarget, GL_TEXTURE_MAX_LEVEL, mipCount);

	glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(glTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLenum glFmt = ogl::GetGLFormat(format), glType = ogl::GetGLType(format);
	unsigned texWidth = texture->GetWidth(), texHeight = texture->GetHeight();
	glTexStorage2D(IF_AND_OR(faceCount > 1, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D), mipCount, GL_RGBA8, texWidth, texHeight);
	for (size_t mip = 0; mip < mipCount; ++mip) {
		for (size_t face = 0; face < faceCount; ++face) {
			size_t index = face * mipCount + mip;
			const Data& data = datas[index];
			//size_t SysMemPitch = data.Size ? data.Size : d3d::BytePerPixel(desc.Format) * (texWidth >> mip);//Line width in bytes
			if (bool compressed = data.Size) {
				glCompressedTexSubImage2D(IF_AND_OR(faceCount > 1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, GL_TEXTURE_2D), mip, 0, 0, texWidth >> mip, texHeight >> mip, format, data.Size, data.Bytes);
			}
			else {
				glTexSubImage2D(IF_AND_OR(faceCount > 1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, GL_TEXTURE_2D), mip, 0, 0, texWidth >> mip, texHeight >> mip, glFmt, glType, data.Bytes);
			}
		}
	}
	
	if (autoGen) {
		for (size_t face = 0; face < faceCount; ++face)
			glGenerateMipmap(IF_AND_OR(faceCount > 1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, GL_TEXTURE_2D));
	}
	glBindTexture(glTarget, 0);
	return texture;
}
void RenderSystemOGL::SetTextures(size_t slot, const ITexturePtr textures[], size_t count)
{
	BOOST_ASSERT(IsCurrentInMainThread());

	for (size_t i = 0; i < count; ++i) {
		glActiveTexture(GL_TEXTURE0 + slot + i);
		TextureOGLPtr tex = std::static_pointer_cast<TextureOGL>(textures[i]);
		glBindTexture(GL_TEXTURE_2D, IF_AND_OR(tex, tex->GetId(), 0));
	}
}
bool RenderSystemOGL::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
{
	return true;
}

ISamplerStatePtr RenderSystemOGL::LoadSampler(IResourcePtr res, const SamplerDesc& desc)
{
	//BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);
	
	SamplerStateOGLPtr sampler = std::static_pointer_cast<SamplerStateOGL>(res);

	GLuint samId = 0;
	glGenSamplers(1, &samId);
	sampler->Init(samId);

	GLenum minFilter, magFilter, compMode;
	std::tuple(minFilter, magFilter, compMode) = ogl::GetGLSamplerFilterMode(desc.Filter);
	glSamplerParameteri(samId, GL_TEXTURE_MIN_FILTER, minFilter);
	glSamplerParameteri(samId, GL_TEXTURE_MAG_FILTER, magFilter);
	glSamplerParameteri(samId, GL_TEXTURE_WRAP_S, ogl::GetGlSamplerAddressMode(desc.AddressU));
	glSamplerParameteri(samId, GL_TEXTURE_WRAP_T, ogl::GetGlSamplerAddressMode(desc.AddressV));
	glSamplerParameteri(samId, GL_TEXTURE_WRAP_R, ogl::GetGlSamplerAddressMode(desc.AddressW));
	float borderColors[4] = {1,1,1,1};
	glSamplerParameterfv(samId, GL_TEXTURE_BORDER_COLOR, borderColors);
	glSamplerParameterf(samId, GL_TEXTURE_MIN_LOD, FLT_MIN);
	glSamplerParameterf(samId, GL_TEXTURE_MAX_LOD, FLT_MAX);
	glSamplerParameterf(samId, GL_TEXTURE_LOD_BIAS, 0.0f);
	glSamplerParameteri(samId, GL_TEXTURE_COMPARE_MODE, compMode);
	glSamplerParameteri(samId, GL_TEXTURE_COMPARE_FUNC, ogl::GetGlCompFunc(desc.CmpFunc));

	return sampler;
}
void RenderSystemOGL::SetSamplers(size_t slot, const ISamplerStatePtr samplers[], size_t count)
{
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(count >= 0);

	for (size_t i = 0; i < count; ++i) {
		SamplerStateOGLPtr sampler = std::static_pointer_cast<SamplerStateOGL>(samplers[i]);
		glBindSampler(slot + i, IF_AND_OR(sampler, sampler->GetId(), 0));
	}
}

void RenderSystemOGL::SetBlendState(const BlendState& blendFunc)
{
	BOOST_ASSERT(IsCurrentInMainThread());

	mCurBlendState = blendFunc;

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(ogl::GetGlBlendFunc(blendFunc.Src), ogl::GetGlBlendFunc(blendFunc.Dst));
}
void RenderSystemOGL::SetDepthState(const DepthState& depthState)
{
	BOOST_ASSERT(IsCurrentInMainThread());

	mCurDepthState = depthState;

	glEnable(GL_DEPTH_TEST);
	glDepthMask(depthState.WriteMask);
	glDepthFunc(ogl::GetGlCompFunc(depthState.CmpFunc));
}

void RenderSystemOGL::DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	BOOST_ASSERT(IsCurrentInMainThread());

	//mDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topo));
	//mDeviceContext->Draw(op.VertexBuffers[0]->GetCount(), 0);
	glDrawArrays(ogl::GetTopologyType(topo), 0, op.VertexBuffers[0]->GetCount());
}
void RenderSystemOGL::DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	BOOST_ASSERT(IsCurrentInMainThread());

	//mDeviceContext->IASetPrimitiveTopology(static_cast<D3D11_PRIMITIVE_TOPOLOGY>(topo));
	int indexCount = op.IndexCount != 0 ? op.IndexCount : op.IndexBuffer->GetBufferSize() / op.IndexBuffer->GetWidth();
	//mDeviceContext->DrawIndexed(indexCount, op.IndexPos, op.IndexBase);
	glDrawElementsBaseVertex(ogl::GetTopologyType(topo), indexCount, ogl::GetGLType(op.IndexBuffer->GetFormat()), (void*)op.IndexPos, op.IndexBase);
}

bool RenderSystemOGL::BeginScene()
{
	return true;
}
void RenderSystemOGL::EndScene(BOOL vsync)
{
	HDC hdc = GetDC(mHWnd);
	SwapBuffers(hdc);
}

}