#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

DECLARE_STRUCT(cbDirectLight);
DECLARE_STRUCT(cbPointLight);
DECLARE_STRUCT(cbSpotLight);

DECLARE_STRUCT(Camera);
DECLARE_STRUCT(SceneManager);

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
DECLARE_STRUCT(IRenderTarget);
DECLARE_STRUCT(ISamplerState);
DECLARE_STRUCT(ConstBufferDeclElement);
DECLARE_STRUCT(ConstBufferDecl);

DECLARE_STRUCT(IRenderSystem);
DECLARE_STRUCT(RenderSystem);
DECLARE_STRUCT(RenderPipeline);

}