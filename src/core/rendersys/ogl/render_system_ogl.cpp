#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/lambda/if.hpp>
#include <regex>
#include "core/mir_config.h"
#include "core/base/debug.h"
#include "core/base/input.h"
#include "core/base/macros.h"
#include "core/base/md5.h"
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

#define BOOST_ASSERT_(X) BOOST_ASSERT(X)
//#define USE_VULKAN_COMPILER 1

namespace mir {

RenderSystemOGL::RenderSystemOGL()
{}
Platform RenderSystemOGL::GetPlatform() const 
{
	return Platform{ kPlatformOpengl, 460 };
}

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
bool RenderSystemOGL::Initialize(HWND hWnd, Eigen::Vector4i viewport)
{
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

	if (!viewport.any()) GetClientRect(mHWnd, (RECT*)&viewport);
	mScreenSize = viewport.tail<2>() - viewport.head<2>();
	_SetViewPort(mCurViewPort = viewport);

	SetFrameBuffer(nullptr);

	CheckHR(glProvokingVertex(GL_FIRST_VERTEX_CONVENTION));
	CheckHR(glDepthRange(0.0f, 1.0f));
	CheckHR(glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE));
	CheckHR(glDisable(GL_PRIMITIVE_RESTART));
	_SetRasterizerState(mCurRasterState = RasterizerState::MakeDefault());
	_SetDepthState(mCurDepthState = DepthState::Make(kCompareLessEqual, kDepthWriteMaskAll, true));
	_SetBlendState(mCurBlendState = BlendState::MakeAlphaNonPremultiplied());
	return true;
}

RenderSystemOGL::~RenderSystemOGL()
{
	Dispose();
}
void RenderSystemOGL::Dispose()
{
	if (mOglCtx) {
		wglDeleteContext(mOglCtx);
		mOglCtx = NULL;
	}
}

bool RenderSystemOGL::IsCurrentInMainThread() const
{
	return mMainThreadId == std::this_thread::get_id();
}
void RenderSystemOGL::UpdateFrame(float dt)
{}

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
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadFrameBuffer");
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);
	BOOST_ASSERT(formats.size() >= 1);

	FrameBufferOGLPtr fbo = std::static_pointer_cast<FrameBufferOGL>(res);
	fbo->Init(size.head<2>());
	BindFrameBuffer bindFrambuffer(GL_FRAMEBUFFER, fbo->GetId());

	int colorCount = IF_AND_OR(IsDepthStencil(formats.back()) || formats.back() == kFormatUnknown, formats.size() - 1, formats.size());
	std::vector<GLenum> buffers(colorCount);
	for (size_t i = 0; i < colorCount; ++i) {
		fbo->SetAttachColor(i, FrameBufferAttachOGLFactory::CreateColorAttachment(size, formats[i]));
		buffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	
	if (colorCount != formats.size()) {
		fbo->SetAttachZStencil(FrameBufferAttachOGLFactory::CreateZStencilAttachment(size.head<2>(), formats.back()));
	}
	
	if (buffers.empty()) buffers.push_back(GL_NONE);
	CheckHR(glDrawBuffers(colorCount, &buffers[0]));

	BOOST_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	return fbo;
}
void RenderSystemOGL::ClearFrameBuffer(IFrameBufferPtr fb, const Eigen::Vector4f& color, float depth, uint8_t stencil)
{
	DEBUG_LOG_CALLSTK("renderSysOGL.ClearFrameBuffer");
	BOOST_ASSERT(IsCurrentInMainThread());
	
	if (mCurRasterState.Scissor.ScissorEnable) CheckHR(glDisable(GL_SCISSOR_TEST));
	if (!mCurDepthState.WriteMask) CheckHR(glDepthMask(GL_TRUE));

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
	if (!mCurDepthState.WriteMask) CheckHR(glDepthMask(GL_FALSE));
}
void RenderSystemOGL::CopyFrameBuffer(IFrameBufferPtr dst, int dstAttachment, IFrameBufferPtr src, int srcAttachment)
{
	DEBUG_LOG_CALLSTK("renderSysOGL.CopyFrameBuffer");
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT((dstAttachment >= 0) == (srcAttachment >= 0));

	FrameBufferOGLPtr srcFb = std::static_pointer_cast<FrameBufferOGL>(src);
	FrameBufferOGLPtr dstFb = std::static_pointer_cast<FrameBufferOGL>(dst);
	GLuint srcFbId = NULLABLE_MEM(srcFb, GetId(), 0);
	GLuint dstFbId = NULLABLE_MEM(dstFb, GetId(), 0);
	Eigen::Vector2i srcSize = NULLABLE_MEM(srcFb, GetSize(), mScreenSize);
	Eigen::Vector2i dstSize = NULLABLE_MEM(dstFb, GetSize(), mScreenSize);

	if (srcAttachment >= 0) {
		CheckHR(glNamedFramebufferReadBuffer(srcFbId, IF_AND_OR(srcFbId, GL_COLOR_ATTACHMENT0 + srcAttachment, GL_BACK)));
		CheckHR(glNamedFramebufferDrawBuffer(dstFbId, IF_AND_OR(dstFbId, GL_COLOR_ATTACHMENT0 + dstAttachment, GL_BACK)));
		CheckHR(glBlitNamedFramebuffer(srcFbId, dstFbId, 0, 0, srcSize.x(), srcSize.y(), 0, 0, dstSize.x(), dstSize.y(), GL_COLOR_BUFFER_BIT, GL_NEAREST));
	}
	else {
		CheckHR(glBlitNamedFramebuffer(srcFbId, dstFbId, 0, 0, srcSize.x(), srcSize.y(), 0, 0, dstSize.x(), dstSize.y(), GL_DEPTH_BUFFER_BIT, GL_NEAREST));
		auto format = srcFb ? srcFb->GetAttachZStencilTexture()->GetFormat() : dstFb->GetAttachZStencilTexture()->GetFormat();
		if (format == kFormatD24UNormS8UInt) CheckHR(glBlitNamedFramebuffer(srcFbId, dstFbId, 0, 0, srcSize.x(), srcSize.y(), 0, 0, dstSize.x(), dstSize.y(), GL_STENCIL_BUFFER_BIT, GL_NEAREST));
	}
}

void RenderSystemOGL::SetFrameBuffer(IFrameBufferPtr fb)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetFrameBuffer");
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
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadLayout");
	BOOST_ASSERT_(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	InputLayoutOGLPtr layout = std::static_pointer_cast<InputLayoutOGL>(res);
	layout->Init(descArr);
	return layout;
}
void RenderSystemOGL::SetVertexLayout(IInputLayoutPtr res)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetVertexLayout");
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

struct ShaderPreprocessor {
public:
	ShaderPreprocessor(const std::string& shaderDir) :mShaderDir(shaderDir) {}
	std::string operator()(const std::string& bin, const std::string& vsOrPsEntry) {
		//TIME_PROFILE("include preprocess");
		std::string src = ReplaceEntry(bin, vsOrPsEntry);
		ProcessInclude(src);
		return std::move(mResult);
	}
	std::vector<char> operator()(const std::vector<char>& bin, const std::string& vsOrPsEntry) {
		//TIME_PROFILE("include preprocess");
		std::string src(bin.begin(), bin.end());
		src = ReplaceEntry(src, vsOrPsEntry);
		ProcessInclude(src);
		return std::vector<char>(mResult.begin(), mResult.end());
	}
private:
	std::string ReplaceEntry(const std::string& src, const std::string& vsOrPsEntry) {
		//TIME_PROFILE("include preprocess: replace entry");
		std::string result;

		std::stringstream ss;
		ss << src;
		std::string line;
		const std::regex vps_regex("[\\s\\t]+void[\\s\\t]+StageEntry_" + vsOrPsEntry + ".*\\r?\\n?");
		BOOL flag = 0;
		while (ss.peek() != EOF) {
			std::getline(ss, line);
			std::smatch exp_match;
			if (line.size() > 10 && (line[0] == ' ' || line[0] == '\t')) {
				if (std::regex_match(line, exp_match, vps_regex)) {
					boost::replace_first(line, "StageEntry_" + vsOrPsEntry, "main");
					flag = TRUE;
				}
			}
			result += line;
			result.push_back('\n');
		}
		BOOST_ASSERT(flag);
		return std::move(result);
	}
	void ProcessInclude(const std::string& src) {
		std::stringstream ss;
		ss << src;
		std::string line;
		static const std::regex exp_regex("#include[\\s\\t]+\"([\\w\\d_\\.]+)\".*\\r?\\n?");
		while (ss.peek() != EOF) {
			std::getline(ss, line);
			std::smatch exp_match;
			if (line.size() > 10 && line[0] == '#' && line[1] == 'i'
				&& std::regex_match(line, exp_match, exp_regex) && exp_match.size() == 2) {
				std::string incname = exp_match[1].str();
				if (mVisits.find(incname) == mVisits.end()) {
					mVisits.insert(incname);

					boost::filesystem::path incpath = boost::filesystem::system_complete(mShaderDir + incname);
					std::vector<char> bin = input::ReadFile(incpath.string().c_str(), "rb");
					BOOST_ASSERT(!bin.empty());
					std::string incstr(&bin[0], bin.size());
					ProcessInclude(incstr);
				}
			}
			else {
				mResult += line;
				mResult.push_back('\n');
			}
		}
	}
private:
	const std::string& mShaderDir;
	std::string mResult;
	std::unordered_set<std::string> mVisits;
};

IBlobDataPtr RenderSystemOGL::CompileShader(const ShaderCompileDesc& compile, const Data& data)
{
	BOOST_ASSERT_(IsCurrentInMainThread());

	BlobDataOGLPtr blob = CreateInstance<BlobDataOGL>();

	blob->mSource = "#version 460 core\n";
	if (!compile.Macros.empty()) {
		for (size_t i = 0; i < compile.Macros.size(); ++i) {
			const auto& cdm = compile.Macros[i];
			blob->mSource += "#define " + cdm.Name + " " + cdm.Definition + "\n";
		}
	}
	blob->mSource += std::string((const char*)data.Bytes, data.Size);

	//preprocess include && entry
	boost::filesystem::path srcPath = compile.SourcePath;
	srcPath.remove_filename();
	std::string shaderDir = srcPath.string();
	if (shaderDir.back() != '/') shaderDir.push_back('/');
	blob->mSource = ShaderPreprocessor(shaderDir)(blob->mSource, compile.EntryPoint);

#if USE_VULKAN_COMPILER
	std::string temp_dir = "temp_spirv/";
	if (! boost::filesystem::is_directory(temp_dir)) {
		boost::filesystem::create_directories(temp_dir);
	}

	uint32_t digest[4];
	md5((const uint8_t*)blob->mSource.c_str(), blob->mSource.length(), (uint8_t*)digest);
	std::string filename = (boost::format("%s%x%x%x%x") %temp_dir %digest[0] %digest[1] %digest[2] %digest[3]).str();
	std::string targetname = filename;
	filename += IF_AND_OR(compile.ShaderType == kShaderVertex, ".glv", ".glf");
	targetname += IF_AND_OR(compile.ShaderType == kShaderVertex, ".spv", ".spf");
	input::WriteFile(filename.c_str(), "wb", blob->mSource.c_str(), blob->mSource.length());

	//ShellExecuteA(mHWnd, "glslc.exe", filename.c_str(), (" -o " + targetname).c_str(), "", SW_HIDE);
	std::string command = (boost::format("glslangValidator.exe -G -S %s %s -o %s") %IF_AND_OR(compile.ShaderType == kShaderVertex, "vert", "frag") %filename %targetname).str();
	int cmdRet = system(command.c_str());
	BOOST_ASSERT(!(cmdRet == 0 && GetLastError() == ENOENT));
	if (cmdRet < 0) {
		MessageBoxA(NULL, command.c_str(), "system command failed", MB_OK);
		return nullptr;
	}
	blob->mBinary = input::ReadFile(targetname.c_str(), "rb");
	BOOST_ASSERT(! blob->mBinary.empty());
#endif
	return blob;
}
IShaderPtr RenderSystemOGL::CreateShader(int type, IBlobDataPtr data)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.CreateShader");
	BOOST_ASSERT_(IsCurrentInMainThread());

	GLuint shaderId = CheckHR(glCreateShader(ogl::GetGLShaderType((ShaderType)type)));
	auto blob = std::static_pointer_cast<BlobDataOGL>(data);
#if !USE_VULKAN_COMPILER
	const char* sources[] = { blob->mSource.c_str() };
	CheckHR(glShaderSource(shaderId, 1, sources, NULL));
	CheckHR(glCompileShader(shaderId));
#else
	BOOST_ASSERT(blob->GetBytes());
	CheckHR(glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, blob->GetBytes(), blob->GetSize()));
	CheckHR(glSpecializeShader(shaderId, "main", 0, nullptr, nullptr));
#endif

#if defined MIR_D3D11_DEBUG
	std::string errMsg;
	if (!ogl::CheckProgramCompileStatus(shaderId, &errMsg)) {
		input::WriteFile("compile.txt", "wb", blob->mSource.c_str(), blob->mSource.length());
		MessageBoxA(NULL, errMsg.c_str(), "opengl compile failed", MB_OK);
		return nullptr;
	}
#else
	if (!ogl::CheckProgramCompileStatus(shaderId, nullptr)) return nullptr;
#endif

	IShaderPtr shader;
	switch (type) {
	case kShaderVertex: {
		shader = CreateInstance<VertexShaderOGL>(data, shaderId);
	}break;
	case kShaderPixel: {
		shader = CreateInstance<PixelShaderOGL>(data, shaderId);
	}break;
	default:
		BOOST_ASSERT(FALSE);
		break;
	}
	return shader;
}
IProgramPtr RenderSystemOGL::LoadProgram(IResourcePtr res, const std::vector<IShaderPtr>& shaders)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadProgram");
	BOOST_ASSERT_(IsCurrentInMainThread());
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
#if defined MIR_D3D11_DEBUG
	std::string errMsg;
	if (!ogl::CheckProgramLinkStatus(proId, &errMsg)) {
		if (auto vertex = program->GetVertex()) {
			const std::string& srcVS = std::static_pointer_cast<BlobDataOGL>(vertex->GetBlob())->mSource;
			input::WriteFile("linkVS.txt", "wb", srcVS.c_str(), srcVS.length());
		}
		if (auto pixel = program->GetPixel()) {
			const std::string& srcPS = std::static_pointer_cast<BlobDataOGL>(pixel->GetBlob())->mSource;
			input::WriteFile("linkPS.txt", "wb", srcPS.c_str(), srcPS.length());
		}
		MessageBoxA(NULL, errMsg.c_str(), "opengl link error", MB_OK);
		return nullptr;
	}
#else
	if (!ogl::CheckProgramLinkStatus(proId)) return nullptr;
#endif

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
	DEBUG_LOG_CALLSTK("renderSysOgl.SetProgram");
	BOOST_ASSERT(IsCurrentInMainThread());

	auto prog = std::static_pointer_cast<ProgramOGL>(program);
	BOOST_ASSERT(ogl::ValidateProgram(prog->GetId()));

	CheckHR(glUseProgram(prog->GetId()));
}

/********** Hardware Buffer **********/
void RenderSystemOGL::SetVertexArray(IVertexArrayPtr vao)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetVertexArray");
	BOOST_ASSERT(IsCurrentInMainThread());

	mCurVao = std::static_pointer_cast<VertexArrayOGL>(vao);
	CheckHR(glBindVertexArray(NULLABLE_MEM(mCurVao, GetId(), 0)));
}

IVertexBufferPtr RenderSystemOGL::LoadVertexBuffer(IResourcePtr res, IVertexArrayPtr ivao, int stride, int offset, const Data& data)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadVertexBuffer");
	BOOST_ASSERT_(IsCurrentInMainThread());
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
	DEBUG_LOG_CALLSTK("renderSysOgl.SetVertexBuffers");
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(count >= 1);

	std::vector<GLuint> vbos(count);
	std::vector<GLintptr> offsets(count);
	std::vector<GLsizei> strides(count);
	for (size_t i = 0; i < count; ++i) {
		mCurVbos[slot + i] = std::static_pointer_cast<VertexBufferOGL>(vertexBuffers[i]);
		auto& vbo = mCurVbos[slot + i];
		if (vbo) {
			BOOST_ASSERT(mCurVao == vbo->GetVAO());
			vbos[i] = vbo->GetId();
			offsets[i] = vbo->GetOffset();
			strides[i] = vbo->GetStride();
		}
	}
	CheckHR(glBindVertexBuffers(slot, count, &vbos[0], &offsets[0], &strides[0]));
}

IIndexBufferPtr RenderSystemOGL::LoadIndexBuffer(IResourcePtr res, IVertexArrayPtr ivao, ResourceFormat format, const Data& data)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadIndexBuffer");
	BOOST_ASSERT_(IsCurrentInMainThread());
	BOOST_ASSERT(res && ivao);

	HWMemoryUsage usage = data.Bytes ? kHWUsageImmutable : kHWUsageDynamic;

	VertexArrayOGLPtr vao = std::static_pointer_cast<VertexArrayOGL>(ivao);
	IndexBufferOGLPtr vio = std::static_pointer_cast<IndexBufferOGL>(res);
	BindVaoScope bindVao(vao->GetId());
	GLuint vioId = 0;
	{
		CheckHR(glGenBuffers(1, &vioId));
		BindVioScope bindVio(vioId);
		CheckHR(glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, data.Size, data.Bytes, IF_AND_OR(usage == kHWUsageDynamic, GL_MAP_WRITE_BIT, 0)));
	}
	vio->Init(std::static_pointer_cast<VertexArrayOGL>(vao), vioId, data.Size, format, usage);
	return vio;
}
void RenderSystemOGL::SetIndexBuffer(IIndexBufferPtr indexBuffer)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetIndexBuffer");
	BOOST_ASSERT(IsCurrentInMainThread());

	mCurVio = std::static_pointer_cast<IndexBufferOGL>(indexBuffer);
	if (mCurVio) {
		BOOST_ASSERT(mCurVao == mCurVio->GetVAO());
		CheckHR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mCurVio->GetId()));
	}
}

IContantBufferPtr RenderSystemOGL::LoadConstBuffer(IResourcePtr res, const ConstBufferDecl& cbDecl, HWMemoryUsage usage, const Data& data)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadConstBuffer");
	BOOST_ASSERT_(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	ContantBufferOGLPtr ubo = std::static_pointer_cast<ContantBufferOGL>(res);
	GLuint uboId = 0;
	GL_CHECK_ERROR();
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
	DEBUG_LOG_CALLSTK("renderSysOgl.SetConstBuffers");
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
	DEBUG_LOG_CALLSTK("renderSysOgl.UpdateBuffer");
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
ITexturePtr RenderSystemOGL::LoadTexture(IResourcePtr res, ResourceFormat format, const Eigen::Vector4i& size/*w_h_step_face*/, int mipCount, const Data2 datas[])
{
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadTexture");
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(res);

	static Data2 defaultData = Data2::MakeNull();
	datas = datas ? datas : &defaultData;
	const HWMemoryUsage usage = datas[0].Bytes ? kHWUsageDefault : kHWUsageDynamic;

	TextureOGLPtr texture = std::static_pointer_cast<TextureOGL>(res);
	texture->Init(format, usage, size.x(), size.y(), size.w(), mipCount);
	BOOST_ASSERT_IF_THEN(texture->GetFaceCount() > 1, datas[0].Bytes);
	BOOST_ASSERT_IF_THEN(texture->IsAutoGenMipmap(), datas[0].Bytes && texture->GetFaceCount() == 1);
	texture->InitTex(datas);
	texture->AutoGenMipmap();
	return texture;
}
bool RenderSystemOGL::LoadRawTextureData(ITexturePtr texture, char* data, int dataSize, int dataStep)
{
	return true;
}
void RenderSystemOGL::GenerateMips(ITexturePtr texture)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.GenerateMips");
	BOOST_ASSERT(IsCurrentInMainThread());
	BOOST_ASSERT(texture);

	std::static_pointer_cast<TextureOGL>(texture)->AutoGenMipmap();
}
void RenderSystemOGL::SetTextures(size_t slot, const ITexturePtr textures[], size_t count)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetTextures");
	BOOST_ASSERT(IsCurrentInMainThread());

	for (size_t i = 0; i < count; ++i) {
		TextureOGLPtr tex = std::static_pointer_cast<TextureOGL>(textures[i]);
		CheckHR(glBindTextureUnit(slot + i, NULLABLE_MEM(tex, GetId(), 0)));
	}
}

ISamplerStatePtr RenderSystemOGL::LoadSampler(IResourcePtr res, const SamplerDesc& desc)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.LoadSampler");
	BOOST_ASSERT_(IsCurrentInMainThread());
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
	DEBUG_LOG_CALLSTK("renderSysOgl.SetSamplers");
	BOOST_ASSERT(IsCurrentInMainThread());

	for (size_t i = 0; i < count; ++i) {
		SamplerStateOGLPtr sampler = std::static_pointer_cast<SamplerStateOGL>(samplers[i]);
		CheckHR(glBindSampler(slot + i, NULLABLE_MEM(sampler, GetId(), 0)));
	}
}

/********** State **********/
void RenderSystemOGL::_SetViewPort(const Eigen::Vector4i& newVp)
{
	CheckHR(glViewport(newVp[0], newVp[1], newVp[2], newVp[3]));
}
void RenderSystemOGL::SetViewPort(int x, int y, int width, int height)
{
	DEBUG_LOG_CALLSTK("renderSysOgl.SetViewPort");
	BOOST_ASSERT(IsCurrentInMainThread());

	Eigen::Vector4i newVP(x, y, width, height);
	if (mCurViewPort != newVP) {
		_SetViewPort(newVP);
		mCurViewPort = newVP;
	}
}

void RenderSystemOGL::_SetBlendState(const BlendState& blendFunc)
{
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
	if (depthState.DepthEnable) {
		CheckHR(glEnable(GL_DEPTH_TEST));
		CheckHR(glDepthFunc(ogl::GetGlCompFunc(depthState.CmpFunc)));
	}
	else {
		CheckHR(glDisable(GL_DEPTH_TEST));
	}
	CheckHR(glDepthMask(depthState.WriteMask));
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
	if (mCurRasterState.Scissor.ScissorEnable != scissor.ScissorEnable) {
		if (scissor.ScissorEnable) { CheckHR(glEnable(GL_SCISSOR_TEST)); }
		else { CheckHR(glDisable(GL_SCISSOR_TEST)); }
	}

	const auto& rct = scissor.Rect;
	OutputDebugStringA((boost::format("glScissor(%d,%d,%d,%d)") %rct[0] %(mCurViewPort[3] - rct[3]) %(rct[2] - rct[0]) %(rct[3] - rct[1])).str().c_str());
	CheckHR(glScissor(rct[0], mCurViewPort[3] - rct[3], rct[2] - rct[0], rct[3] - rct[1]));
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
	_SetCullMode(rs.CullMode);
	_SetFillMode(rs.FillMode);
	_SetDepthBias(rs.DepthBias);
	_SetScissorState(rs.Scissor);
}

/********** Draw **********/
void RenderSystemOGL::DrawPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	DEBUG_LOG_CALLSTK("renderSysOgl.DrawPrimitive");
	BOOST_ASSERT(IsCurrentInMainThread());

	CheckHR(glDrawArrays(ogl::GetGLTopologyType(topo), 0, op.VertexBuffers[0]->GetCount()));
}
void RenderSystemOGL::DrawIndexedPrimitive(const RenderOperation& op, PrimitiveTopology topo) {
	DEBUG_LOG_CALLSTK("renderSysOgl.DrawIndexedPrimitive");
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
	BOOST_ASSERT(IsCurrentInMainThread());

	HDC hdc = GetDC(mHWnd);
	SwapBuffers(hdc);
}

}