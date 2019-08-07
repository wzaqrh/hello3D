#pragma once
#include "std.h"

#define USE_ONLY_PNG
#define USE_RENDER_OP
#define D3D11_DEBUG
#define PRELOAD_SHADER

class TRenderSystem;

//BaseType.h
struct TCamera;
typedef std::shared_ptr<TCamera> TCameraPtr;

class TDirectLight;
typedef std::shared_ptr<TDirectLight> TDirectLightPtr;

class TPointLight;
typedef std::shared_ptr<TPointLight> TPointLightPtr;

class TSpotLight;
typedef std::shared_ptr<TSpotLight> TSpotLightPtr;

//TInterfaceType.h
struct IResource;

struct TVertexShader;
typedef std::shared_ptr<TVertexShader> TVertexShaderPtr;

struct TPixelShader;
typedef std::shared_ptr<TPixelShader> TPixelShaderPtr;

struct TProgram;
typedef std::shared_ptr<TProgram> TProgramPtr;

struct TInputLayout;
typedef std::shared_ptr<TInputLayout> TInputLayoutPtr;

struct THardwareBuffer;

struct TVertexBuffer;
typedef std::shared_ptr<TVertexBuffer> TVertexBufferPtr;

struct TIndexBuffer;
typedef std::shared_ptr<TIndexBuffer> TIndexBufferPtr;

struct TContantBuffer;
typedef std::shared_ptr<TContantBuffer> TContantBufferPtr;

struct TTexture;
typedef std::shared_ptr<TTexture> TTexturePtr;

struct TTextureBySlot;
typedef std::shared_ptr<TTextureBySlot> TTextureBySlotPtr;

class TRenderTexture;
typedef std::shared_ptr<TRenderTexture> TRenderTexturePtr;

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
class SDTimer;
typedef std::shared_ptr<class SDTimer> SDTimerPtr;

typedef std::function<void(IResource* ress, HRESULT hr)> TThreadPumpCallback;