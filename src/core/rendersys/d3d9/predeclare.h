#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

DECLARE_STRUCT(BlobData9);
DECLARE_STRUCT(InputLayout9);
DECLARE_STRUCT(VertexShader9);
DECLARE_STRUCT(PixelShader9);
DECLARE_STRUCT(ConstantTable);
DECLARE_STRUCT(Program9);
DECLARE_STRUCT(VertexBuffer9);
DECLARE_STRUCT(IndexBuffer9);
DECLARE_STRUCT(ContantBuffer9);
DECLARE_STRUCT(Texture9);
DECLARE_STRUCT(FrameBuffer9);
DECLARE_STRUCT(SamplerState9);

}