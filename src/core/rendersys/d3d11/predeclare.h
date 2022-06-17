#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

DECLARE_STRUCT(BlobData11);
DECLARE_STRUCT(InputLayout11);
DECLARE_STRUCT(VertexShader11);
DECLARE_STRUCT(PixelShader11);
DECLARE_STRUCT(Program11);
DECLARE_STRUCT(VertexArray11);
DECLARE_STRUCT(VertexBuffer11);
DECLARE_STRUCT(IndexBuffer11);
DECLARE_STRUCT(ContantBuffer11);
DECLARE_STRUCT(Texture11);
DECLARE_STRUCT(FrameBuffer11);
DECLARE_STRUCT(SamplerState11);

}