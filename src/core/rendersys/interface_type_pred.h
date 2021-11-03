#pragma once
#include "core/base/std.h"

struct IRenderSystem;
#ifdef USE_EXPORT_COM

typedef ComPtr<struct IBlobData> IBlobDataPtr;
typedef ComPtr<struct IInputLayout> IInputLayoutPtr;
typedef ComPtr<struct IVertexShader> IVertexShaderPtr;
typedef ComPtr<struct IPixelShader> IPixelShaderPtr;
typedef ComPtr<struct IProgram> IProgramPtr;
typedef ComPtr<struct IHardwareBuffer> IHardwareBufferPtr;
typedef ComPtr<struct IVertexBuffer> IVertexBufferPtr;
typedef ComPtr<struct IIndexBuffer> IIndexBufferPtr;
typedef ComPtr<struct IContantBuffer> IContantBufferPtr;
typedef ComPtr<struct ITexture> ITexturePtr;
typedef ComPtr<struct IRenderTexture> IRenderTexturePtr;
typedef ComPtr<struct ISamplerState> ISamplerStatePtr;
typedef ComPtr<class TThreadPump> TThreadPumpPtr;
struct TConstBufferDeclElement;
typedef std::shared_ptr<struct TConstBufferDecl> TConstBufferDeclPtr;
#else

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
typedef std::shared_ptr<class TThreadPump> TThreadPumpPtr;
struct TConstBufferDeclElement;
typedef std::shared_ptr<struct TConstBufferDecl> TConstBufferDeclPtr;
#endif

#ifdef USE_EXPORT_COM
typedef ComPtr<struct IResource> IResourcePtr;
typedef ComPtr<struct TResource> TResourcePtr;
#else
typedef std::shared_ptr<struct IResource> IResourcePtr;
typedef std::shared_ptr<struct TResource> TResourcePtr;
#endif