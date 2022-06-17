#pragma once
#include <windows.h>
#include <glad/glad.h>
#include "core/rendersys/base/res_format.h"
#include "core/rendersys/base/blend_state.h"
#include "core/rendersys/base/compare_func.h"
#include "core/rendersys/base/primitive_topology.h"
#include "core/rendersys/sampler.h"

namespace mir {
namespace ogl {

GLenum GetGLFormat(ResourceFormat fmt);
GLenum GetGLType(ResourceFormat fmt);
bool IsNormalized(ResourceFormat fmt);
size_t GetChannelCount(ResourceFormat fmt);

std::tuple<GLenum, GLenum, GLenum> GetGLSamplerFilterMode(SamplerFilterMode sfmode);
GLenum GetGlSamplerAddressMode(AddressMode addressMode);
GLenum GetGlCompFunc(CompareFunc compFunc);

GLenum GetGlBlendFunc(BlendFunc func);
GLenum GetGlBlendOp();

GLenum GetShaderType(int type);

GLenum GLCheckError(const char* file, int line);
#define GL_CHECK_ERROR() GLCheckError(__FILE__, __LINE__)

GLenum GetTopologyType(PrimitiveTopology topo);
}
}