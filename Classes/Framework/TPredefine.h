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
typedef ComPtr<IVertexShader> IVertexShaderPtr;

struct IPixelShader;
typedef ComPtr<IPixelShader> IPixelShaderPtr;

struct TProgram;
typedef ComPtr<TProgram> TProgramPtr;

struct IInputLayout;
typedef ComPtr<IInputLayout> IInputLayoutPtr;

struct IHardwareBuffer;
struct THardwareBuffer;

struct IVertexBuffer;
typedef ComPtr<IVertexBuffer> IVertexBufferPtr;

struct IIndexBuffer;
typedef ComPtr<IIndexBuffer> IIndexBufferPtr;

struct IContantBuffer;
typedef ComPtr<IContantBuffer> IContantBufferPtr;

struct ITexture;
typedef ComPtr<ITexture> ITexturePtr;

struct IRenderTexture;
typedef ComPtr<IRenderTexture> IRenderTexturePtr;

struct ISamplerState;
typedef ComPtr<ISamplerState> ISamplerStatePtr;

/////
struct TTextureBySlot;
typedef std::shared_ptr<TTextureBySlot> TTextureBySlotPtr;

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