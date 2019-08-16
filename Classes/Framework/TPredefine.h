#pragma once
#include "std.h"

#define USE_ONLY_PNG
#define USE_RENDER_OP
#define D3D11_DEBUG
//#define PRELOAD_SHADER

class IRenderSystem;

//BaseType.h
struct TCamera;
typedef std::shared_ptr<TCamera> TCameraPtr;

class TDirectLight;
typedef std::shared_ptr<TDirectLight> TDirectLightPtr;

class TPointLight;
typedef std::shared_ptr<TPointLight> TPointLightPtr;

class TSpotLight;
typedef std::shared_ptr<TSpotLight> TSpotLightPtr;

struct cbGlobalParam;

struct TConstBufferDeclElement;
struct TConstBufferDecl;
typedef std::shared_ptr<TConstBufferDecl> TConstBufferDeclPtr;
//TInterfaceType.h
struct IResource;

struct IVertexShader;
typedef std::shared_ptr<IVertexShader> IVertexShaderPtr;

struct TVertexShader9;
typedef std::shared_ptr<TVertexShader9> TVertexShader9Ptr;

struct TVertexShader11;
typedef std::shared_ptr<TVertexShader11> TVertexShader11Ptr;

struct IPixelShader;
typedef std::shared_ptr<IPixelShader> IPixelShaderPtr;

struct TPixelShader9;
typedef std::shared_ptr<TPixelShader9> TPixelShader9Ptr;

struct TPixelShader11;
typedef std::shared_ptr<TPixelShader11> TPixelShader11Ptr;

struct TProgram;
typedef std::shared_ptr<TProgram> TProgramPtr;

struct IInputLayout;
typedef std::shared_ptr<IInputLayout> IInputLayoutPtr;

struct TInputLayout9;
typedef std::shared_ptr<TInputLayout9> TInputLayout9Ptr;

struct TInputLayout11;
typedef std::shared_ptr<TInputLayout11> TInputLayoutPtr;

struct IHardwareBuffer;
struct THardwareBuffer;

struct IVertexBuffer;
typedef std::shared_ptr<IVertexBuffer> IVertexBufferPtr;

struct TVertexBuffer9;
typedef std::shared_ptr<TVertexBuffer9> TVertexBuffer9Ptr;

struct TVertexBuffer11;
typedef std::shared_ptr<TVertexBuffer11> TVertexBuffer11Ptr;

struct IIndexBuffer;
typedef std::shared_ptr<IIndexBuffer> IIndexBufferPtr;

struct TIndexBuffer11;
typedef std::shared_ptr<TIndexBuffer11> TIndexBufferPtr;

struct IContantBuffer;
typedef std::shared_ptr<IContantBuffer> IContantBufferPtr;

struct TContantBuffer9;
typedef std::shared_ptr<TContantBuffer9> TContantBuffer9Ptr;

struct TContantBuffer11;
typedef std::shared_ptr<TContantBuffer11> TContantBuffer11Ptr;

struct ITexture;
typedef std::shared_ptr<ITexture> ITexturePtr;

struct TTexture11;
typedef std::shared_ptr<TTexture11> TTexture11Ptr;

struct TTextureBySlot;
typedef std::shared_ptr<TTextureBySlot> TTextureBySlotPtr;

struct IRenderTexture;
typedef std::shared_ptr<IRenderTexture> IRenderTexturePtr;

class TRenderTexture11;
typedef std::shared_ptr<TRenderTexture11> TRenderTexturePtr;

class TRenderTexture9;
typedef std::shared_ptr<TRenderTexture9> TRenderTexture9Ptr;

struct ISamplerState;
typedef std::shared_ptr<ISamplerState> ISamplerStatePtr;

struct TSamplerState9;
typedef std::shared_ptr<TSamplerState9> TSamplerState9Ptr;

struct TSamplerState11;
typedef std::shared_ptr<TSamplerState11> TSamplerState11Ptr;

struct TRenderOperation;
struct TRenderOperationQueue;
struct IRenderable;

//TMaterial.h
struct TPass;
typedef std::shared_ptr<TPass> TPassPtr;

struct TTechnique;
typedef std::shared_ptr<TTechnique> TTechniquePtr;

struct TMaterial;
typedef std::shared_ptr<TMaterial> TMaterialPtr;

struct TMaterialBuilder;

struct TMaterialFactory;
typedef std::shared_ptr<TMaterialFactory> TMaterialFactoryPtr;

typedef std::shared_ptr<struct TMovable> TMovablePtr;
typedef std::shared_ptr<class TSkyBox> TSkyBoxPtr;
typedef std::shared_ptr<class TPostProcess> TPostProcessPtr;
typedef std::shared_ptr<class TThreadPump> TThreadPumpPtr;
typedef std::shared_ptr<class TSprite> TSpritePtr;

//Utility.h
struct TData;
class SDTimer;
typedef std::shared_ptr<class SDTimer> SDTimerPtr;

typedef std::function<void(IResource* ress, HRESULT hr)> TThreadPumpCallback;