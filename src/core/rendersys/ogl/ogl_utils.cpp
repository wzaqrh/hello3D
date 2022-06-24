#include <map>
#include <boost/format.hpp>
#include <d3d11.h>
#include <d3d9.h>
#include "core/base/debug.h"
#include "core/base/input.h"
#include "core/rendersys/ogl/ogl_utils.h"
#include "core/rendersys/program.h"

namespace mir {
namespace ogl {

GLFormatInfo GetGlFormatInfo(ResourceFormat fmt)
{
	gli::dx::dxgi_format_dds dxgiFmt = (gli::dx::dxgi_format_dds)fmt;
	static gli::dx dx1;
	gli::format gliFmt = dx1.find(gli::dx::D3DFMT_DX10, gli::dx::dxgiFormat(dxgiFmt));

	static gli::gl gl1(gli::gl::PROFILE_GL33);
	static gli::swizzles defSwizzles = gli::swizzles(gli::gl::SWIZZLE_RED, gli::gl::SWIZZLE_GREEN, gli::gl::SWIZZLE_BLUE, gli::gl::SWIZZLE_ALPHA);
	gli::gl::format glFmt = gl1.translate(gliFmt, defSwizzles);
	GLFormatInfo res;
	res.InternalFormat = glFmt.Internal;
	res.ExternalFormat = glFmt.External;
	res.InternalType = glFmt.Type;
	res.IsNormalized = gli::is_normalized(gliFmt);
	res.IsCompressed = gli::is_compressed(gliFmt);
	res.IsSRgb = gli::is_srgb(gliFmt);
	res.ChannelCount = gli::component_count(gliFmt);
	return res;
}

std::tuple<GLenum, GLenum, GLenum> GetGLSamplerFilterMode(SamplerFilterMode sfmode)
{
	GLenum minFilter = GL_NEAREST, magFilter = GL_NEAREST, compMode = GL_NONE;

	switch (sfmode)
	{
	case kSamplerFilterMinMagMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinMagPointMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinPointMagLinearMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterMinPointMagMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterMinLinearMagMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinLinearMagPointMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		break;
	case kSamplerFilterMinMagLinearMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterMinMagMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterAnisotropic:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		break;
	case kSamplerFilterCmpMinMagMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinMagPointMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinPointMagLinearMipPoint:
		minFilter = GL_NEAREST_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinPointMagMipLinear:
		minFilter = GL_NEAREST_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinLinearMagMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinLinearMagPointMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_NEAREST;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinMagLinearMipPoint:
		minFilter = GL_LINEAR_MIPMAP_NEAREST;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpMinMagMipLinear:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	case kSamplerFilterCmpAnisotropic:
		minFilter = GL_LINEAR_MIPMAP_LINEAR;
		magFilter = GL_LINEAR;
		compMode = GL_COMPARE_R_TO_TEXTURE;
		break;
	default:
		break;
	}

	return std::make_tuple(minFilter, magFilter, compMode);
}

GLenum GetGlCompFunc(CompareFunc compFunc)
{
	GLenum glCompFunc = GL_NONE;
	switch (compFunc)
	{
	case kCompareNever:
		glCompFunc = GL_NEVER;
		break;
	case kCompareLess:
		glCompFunc = GL_LESS;
		break;
	case kCompareEqual:
		glCompFunc = GL_EQUAL;
		break;
	case kCompareLessEqual:
		glCompFunc = GL_LEQUAL;
		break;
	case kCompareGreater:
		glCompFunc = GL_GREATER;
		break;
	case kCompareNotEqual:
		glCompFunc = GL_NOTEQUAL;
		break;
	case kCompareGreaterEqual:
		glCompFunc = GL_GEQUAL;
		break;
	case kCompareAlways:
		glCompFunc = GL_ALWAYS;
		break;
	case kCompareUnkown:
	default:
		break;
	}
	return glCompFunc;
}

GLenum GetGlSamplerAddressMode(AddressMode addressMode)
{
	GLenum glAddrMode = GL_NONE;
	switch (addressMode)
	{
	case kAddressWrap:
		glAddrMode = GL_REPEAT;
		break;
	case kAddressMirror:
		glAddrMode = GL_MIRRORED_REPEAT;
		break;
	case kAddressClamp:
		glAddrMode = GL_CLAMP_TO_EDGE;
		break;
	case kAddressBorder:
		glAddrMode = GL_CLAMP_TO_BORDER;
		break;
	case kAddressMirrorOnce:
		glAddrMode = GL_MIRROR_CLAMP_TO_EDGE;
		break;
	case kAddressUnkown:
	default:
		break;
	}
	return glAddrMode;
}

GLenum GetGlBlendFunc(BlendFunc func)
{
	GLenum glFunc = GL_NONE;
	switch (func)
	{
	case kBlendZero:
		glFunc = GL_ZERO;
		break;
	case kBlendOne:
		glFunc = GL_ONE;
		break;
	case kBlendSrcColor:
		glFunc = GL_SRC_COLOR;
		break;
	case kBlendInvSrcColor:
		glFunc = GL_ONE_MINUS_SRC_COLOR;
		break;
	case kBlendSrcAlpha:
		glFunc = GL_SRC_ALPHA;
		break;
	case kBlendInvSrcAlpha:
		glFunc = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case kBlendDstAlpha:
		glFunc = GL_DST_ALPHA;
		break;
	case kBlendInvDstAlpha:
		glFunc = GL_ONE_MINUS_DST_ALPHA;
		break;
	case kBlendDstColor:
		glFunc = GL_DST_COLOR;
		break;
	case kBlendInvDstColor:
		glFunc = GL_ONE_MINUS_DST_COLOR;
		break;
	case kBlendSrcAlphaSat:
		glFunc = GL_SRC_ALPHA_SATURATE;
		break;
	case kBlendBlendFactor:
		glFunc = GL_CONSTANT_COLOR;
		break;
	case kBlendInvBlendFactor:
		glFunc = GL_ONE_MINUS_CONSTANT_COLOR;
		break;
	case kBlendSrc1Color:
		glFunc = GL_SRC1_COLOR;
		break;
	case kBlendInvSrc1Color:
		glFunc = GL_ONE_MINUS_SRC1_COLOR;
		break;
	case kBlendSrc1Alpha:
		glFunc = GL_SRC1_ALPHA;
		break;
	case kBlendInvSrc1Alpha:
		glFunc = GL_ONE_MINUS_SRC1_ALPHA;
		break;
	default:
		break;
	}
	return glFunc;
}

GLenum ogl::GetGLShaderType(int type)
{
	GLenum glType = GL_NONE;
	switch (type)
	{
	case kShaderVertex:
		glType = GL_VERTEX_SHADER;
		break;
	case kShaderPixel:
		glType = GL_FRAGMENT_SHADER;
		break;
	default:
		break;
	}
	return glType;
}

GLenum GetGLTopologyType(PrimitiveTopology topo)
{
	GLenum glTopo = GL_NONE;
	switch (topo)
	{
	case kPrimTopologyPointList: glTopo = GL_POINTS; break;

	case kPrimTopologyLineList: glTopo = GL_LINES; break;
	case kPrimTopologyLineStrip: glTopo = GL_LINE_STRIP; break;

	case kPrimTopologyTriangleList: glTopo = GL_TRIANGLES; break;
	case kPrimTopologyTriangleStrip: glTopo = GL_TRIANGLE_STRIP; break;

	case kPrimTopologyLineListAdj: glTopo = GL_LINES_ADJACENCY; break;
	case kPrimTopologyLineStripAdj: glTopo = GL_LINE_STRIP_ADJACENCY; break;

	case kPrimTopologyTriangleListAdj: glTopo = GL_TRIANGLES_ADJACENCY; break;
	case kPrimTopologyTriangleStripAdj: glTopo = GL_TRIANGLE_STRIP_ADJACENCY; break;

	case kPrimTopologyUnkown:
	default: break;
	}
	return glTopo;
}

GLenum GLCheckError(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		//std::cout << error << " | " << file << " (" << line << ")" << std::endl;
		DEBUG_LOG_ERROR((boost::format("%s: %s(%d)") % error % file % line).str().c_str());
		MessageBoxA(NULL, (boost::format("%s: %s(%d)") % error % file % line).str().c_str(), "opengl failed", MB_OK);
	}
	return errorCode;
}

bool ValidateProgram(GLuint proId)
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
		MessageBoxA(NULL, (boost::format("%s\n") % &errorLog[0]).str().c_str(), "opengl validate program failed", MB_OK);
	}
	return validateRes == GL_TRUE;
}

bool CheckProgramCompileStatus(GLuint shaderId, std::string* pErrMsg)
{
	GLint compileState = GL_FALSE;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileState);
	if (!compileState) {
		int logLength;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<char> errorLog(logLength);
		glGetShaderInfoLog(shaderId, logLength, NULL, &errorLog[0]);
		std::string errMsg = (boost::format("%s\n") % &errorLog[0]).str();
		if (pErrMsg) *pErrMsg = errMsg;

		DEBUG_LOG_ERROR(errMsg.c_str());
	}
	return compileState;
}

bool CheckProgramLinkStatus(GLuint progId, std::string* pErrMsg)
{
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(progId, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		int logLength;
		glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<char> errorLog(logLength);
		glGetProgramInfoLog(progId, logLength, NULL, &errorLog[0]);
		std::string errMsg = (boost::format("%s\n") % &errorLog[0]).str();
		if (pErrMsg) *pErrMsg = errMsg;

		DEBUG_LOG_ERROR(errMsg.c_str());
	}
	return linkStatus;
}

}
}