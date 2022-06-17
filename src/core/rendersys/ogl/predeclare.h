#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

DECLARE_STRUCT(OglCaps);
DECLARE_STRUCT(BlobDataOGL);
DECLARE_STRUCT(InputLayoutOGL);
DECLARE_STRUCT(VertexShaderOGL);
DECLARE_STRUCT(PixelShaderOGL);
DECLARE_STRUCT(ProgramOGL);
DECLARE_STRUCT(VertexArrayOGL);
DECLARE_STRUCT(VertexBufferOGL);
DECLARE_STRUCT(IndexBufferOGL);
DECLARE_STRUCT(ContantBufferOGL);
DECLARE_STRUCT(TextureOGL);
DECLARE_STRUCT(FrameBufferOGL);
DECLARE_STRUCT(SamplerStateOGL);

}