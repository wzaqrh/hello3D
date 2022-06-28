#pragma once
#include <windows.h>
#include <glad/glad.h>
#include <gli/gl.hpp>
#include <gli/dx.hpp>
#include "core/mir_config.h"
#include "core/rendersys/base/res_format.h"
#include "core/rendersys/base/blend_state.h"
#include "core/rendersys/base/compare_func.h"
#include "core/rendersys/base/primitive_topology.h"
#include "core/rendersys/sampler.h"

namespace mir {
namespace ogl {

struct GLFormatInfo {
	gli::gl::internal_format InternalFormat;
	gli::gl::external_format ExternalFormat;
	gli::gl::type_format InternalType;
	size_t ChannelCount;
	bool IsNormalized;
	bool IsCompressed;
	bool IsSRgb;
};
GLFormatInfo GetGlFormatInfo(ResourceFormat fmt);

std::tuple<GLenum, GLenum, GLenum> GetGLSamplerFilterMode(SamplerFilterMode sfmode);
GLenum GetGlSamplerAddressMode(AddressMode addressMode);
GLenum GetGlCompFunc(CompareFunc compFunc);

GLenum GetGlBlendFunc(BlendFunc func);
GLenum GetGlBlendOp();

GLenum GetGLShaderType(int type);

GLenum GetGLTopologyType(PrimitiveTopology topo);

GLenum GLCheckError(const char* file, int line);

bool ValidateProgram(GLuint proId);
bool CheckProgramCompileStatus(GLuint shaderId, std::string* pErrMsg);
bool CheckProgramLinkStatus(GLuint proId, std::string* pErrMsg);

}
}

#if defined MIR_RENDERSYS_DEBUG
#define GL_CHECK_ERROR()  mir::ogl::GLCheckError(__FILE__, __LINE__)
#define CheckHR(Sentense) Sentense; GL_CHECK_ERROR()
#else
#define GL_CHECK_ERROR()
#define CheckHR(Sentense) Sentense
#endif