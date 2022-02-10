#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr; typedef std::weak_ptr<TYPE> TYPE##WeakPtr;  
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr; typedef std::weak_ptr<TYPE> TYPE##WeakPtr;

DECLARE_STRUCT(CbDeclElement);
DECLARE_STRUCT(ConstBufferDecl);

DECLARE_STRUCT(IBlobData);
DECLARE_STRUCT(IInputLayout);
DECLARE_STRUCT(IShader);
DECLARE_STRUCT(IVertexShader);
DECLARE_STRUCT(IPixelShader);
DECLARE_STRUCT(IProgram);
DECLARE_STRUCT(IHardwareBuffer);
DECLARE_STRUCT(IVertexBuffer);
DECLARE_STRUCT(IIndexBuffer);
DECLARE_STRUCT(IContantBuffer);
DECLARE_STRUCT(ITexture);
DECLARE_STRUCT(IFrameBufferAttachment);
DECLARE_STRUCT(IFrameBuffer);
DECLARE_STRUCT(ISamplerState);

DECLARE_STRUCT(IRenderSystem);
DECLARE_STRUCT(RenderSystem);
DECLARE_STRUCT(RenderPipeline);
DECLARE_STRUCT(RenderStatesBlock);
}