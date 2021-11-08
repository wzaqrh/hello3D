#pragma once
#include "core/base/std.h"

namespace mir {
#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr 
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr

DECLARE_STRUCT(Transform);
DECLARE_STRUCT(Movable);

DECLARE_STRUCT(IBlobData);
DECLARE_STRUCT(IInputLayout);
DECLARE_STRUCT(IVertexShader);
DECLARE_STRUCT(IPixelShader);
DECLARE_STRUCT(IProgram);
DECLARE_STRUCT(IHardwareBuffer);
DECLARE_STRUCT(IVertexBuffer);
DECLARE_STRUCT(IIndexBuffer);
DECLARE_STRUCT(IContantBuffer);
DECLARE_STRUCT(ITexture);
DECLARE_STRUCT(IRenderTexture);
DECLARE_STRUCT(ISamplerState);
DECLARE_CLASS(ThreadPump);
DECLARE_STRUCT(ConstBufferDeclElement);
DECLARE_STRUCT(ConstBufferDecl);

DECLARE_STRUCT(IRenderSystem);
DECLARE_STRUCT(RenderSystem);

DECLARE_STRUCT(IResource);
DECLARE_STRUCT(Resource);

DECLARE_STRUCT(cbDirectLight);
DECLARE_STRUCT(cbPointLight);
DECLARE_STRUCT(cbSpotLight);
DECLARE_STRUCT(cbGlobalParam);

DECLARE_STRUCT(Pass);
DECLARE_STRUCT(Technique);
DECLARE_STRUCT(Material);
DECLARE_STRUCT(MaterialFactory);

DECLARE_STRUCT(CameraBase);
DECLARE_STRUCT(Camera);
DECLARE_STRUCT(SceneManager);

}