#pragma once
#include "core/base/std.h"

namespace mir {

struct IRenderSystem;

typedef std::shared_ptr<struct IBlobData> IBlobDataPtr;
typedef std::shared_ptr<struct IInputLayout> IInputLayoutPtr;
typedef std::shared_ptr<struct IVertexShader> IVertexShaderPtr;
typedef std::shared_ptr<struct IPixelShader> IPixelShaderPtr;
typedef std::shared_ptr<struct IProgram> IProgramPtr;
typedef std::shared_ptr<struct IHardwareBuffer> IHardwareBufferPtr;
typedef std::shared_ptr<struct IVertexBuffer> IVertexBufferPtr;
typedef std::shared_ptr<struct IIndexBuffer> IIndexBufferPtr;
typedef std::shared_ptr<struct IContantBuffer> IContantBufferPtr;
typedef std::shared_ptr<struct ITexture> ITexturePtr;
typedef std::shared_ptr<struct IRenderTexture> IRenderTexturePtr;
typedef std::shared_ptr<struct ISamplerState> ISamplerStatePtr;
typedef std::shared_ptr<class TThreadPump> ThreadPumpPtr;
struct ConstBufferDeclElement;
typedef std::shared_ptr<struct ConstBufferDecl> TConstBufferDeclPtr;

typedef std::shared_ptr<struct IResource> IResourcePtr;
typedef std::shared_ptr<struct Resource> TResourcePtr;

}