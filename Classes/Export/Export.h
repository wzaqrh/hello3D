#pragma once
#include "std.h"
#include "TPredefine.h"
#include "IRenderSystem.h"
#include "TSprite.h"

#define DLL_EXPORT extern "C" __declspec(dllexport)

#ifdef EXPORT_STRUCT
#pragma pack(push,1)
struct ExportRenderable {
	IRenderable* self;
	ExportRenderable(IRenderable* self__ = nullptr) :self(self__) {}
	IRenderable* operator->() { return self; }
};
struct ExportRenderSystem {
	IRenderSystem* self;
	ExportRenderSystem(IRenderSystem* self__ = nullptr) :self(self__) {}
	IRenderSystem* operator->() { return self; }
};
struct ExportTexture {
	ITexture* self;
	ExportTexture(ITexture* self__ = nullptr) :self(self__) {}
	ITexture* operator->() { return self; }
};
struct ExportSprite : ExportRenderable {
	ExportSprite(TSprite* self__ = nullptr) :ExportRenderable(self__) {}
	TSprite* operator->() { return static_cast<TSprite*>(self); }
	void DestroySelf() { delete self; }
};
#pragma pack(pop)
#else
typedef IRenderable* ExportRenderable;
typedef IRenderSystem* ExportRenderSystem;
typedef ITexture* ExportTexture;
typedef TSprite* ExportSprite;
typedef IRenderTexture* ExportRenderTarget;
#endif

//RenderSystem
DLL_EXPORT ExportRenderSystem RenderSystem_Create(HWND hWnd, bool isd3d11);
DLL_EXPORT void RenderSystem_Destroy(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_Render(ExportRenderSystem rendersys, XMFLOAT4 bgColor, ExportRenderable* renderables, int renderableCount);
DLL_EXPORT void RenderSystem_RClear(ExportRenderSystem rendersys, XMFLOAT4 bgColor);
DLL_EXPORT bool RenderSystem_RBeginScene(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_RRender(ExportRenderSystem rendersys, ExportRenderable* renderables, int renderableCount);
DLL_EXPORT void RenderSystem_REndScene(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_SetRenderTarget(ExportRenderSystem rendersys, ExportRenderTarget renderTarget);

//RenderTarget
DLL_EXPORT ExportRenderTarget RenderTarget_Create(ExportRenderSystem rendersys, int width, int height, DXGI_FORMAT format);
DLL_EXPORT ExportTexture RenderTarget_GetTexture(ExportRenderTarget renderTarget);

//Texture
DLL_EXPORT ExportTexture Texture_Load(ExportRenderSystem rendersys, const char* imgPath, bool async);
DLL_EXPORT ExportTexture Texture_Create(ExportRenderSystem rendersys, int width, int height, DXGI_FORMAT format, int mipmap);
DLL_EXPORT bool Texture_LoadRawData(ExportRenderSystem rendersys, ExportTexture texture, PBYTE data, int dataSize, int dataStep);
DLL_EXPORT int Texture_Width(ExportTexture texture);
DLL_EXPORT int Texture_Height(ExportTexture texture);
DLL_EXPORT DXGI_FORMAT Texture_Format(ExportTexture texture);
DLL_EXPORT int Texture_MipmapCount(ExportTexture texture);

//TSprite
DLL_EXPORT ExportSprite SpriteColor_Create(ExportRenderSystem rendersys, XMFLOAT4 color);
DLL_EXPORT ExportSprite SpriteImage_Create(ExportRenderSystem rendersys, const char* imgPath);
DLL_EXPORT void Sprite_Destroy(ExportSprite sprite);
DLL_EXPORT void Sprite_SetTexture(ExportSprite sprite, ExportTexture texture);
DLL_EXPORT void Sprite_SetRect(ExportSprite sprite, XMFLOAT2 pos, XMFLOAT2 size);
DLL_EXPORT void Sprite_SetColor(ExportSprite sprite, XMFLOAT4 color);
DLL_EXPORT void Sprite_SetFlipY(ExportSprite sprite, bool flipY);
 