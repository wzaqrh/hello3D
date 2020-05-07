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
#endif

DLL_EXPORT LONG JustTest(LONG p1, LONG p2);

//RenderSystem
DLL_EXPORT ExportRenderSystem RenderSystem_Create(HWND hWnd, bool isd3d11);
DLL_EXPORT void RenderSystem_Destroy(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_Render(ExportRenderSystem rendersys, XMFLOAT4 bgColor, ExportRenderable* renderables, int renderableCount);

//Texture
DLL_EXPORT ExportTexture Texture_Load(ExportRenderSystem rendersys, const char* imgPath);
DLL_EXPORT ExportTexture Texture_Create(ExportRenderSystem rendersys, int width, int height, DXGI_FORMAT format, int mipmap);
DLL_EXPORT bool Texture_LoadRawData(ExportRenderSystem rendersys, ExportTexture texture, PBYTE data, int dataSize, int dataStep);
DLL_EXPORT int Texture_Width(ExportTexture texture);
DLL_EXPORT int Texture_Height(ExportTexture texture);
DLL_EXPORT DXGI_FORMAT Texture_Format(ExportTexture texture);
DLL_EXPORT int Texture_MipmapCount(ExportTexture texture);

//TSprite
DLL_EXPORT ExportSprite Sprite_Create(ExportRenderSystem rendersys, const char* imgPath);
DLL_EXPORT void Sprite_Destroy(ExportSprite sprite);
DLL_EXPORT void Sprite_SetTexture(ExportSprite sprite, ExportTexture texture);
DLL_EXPORT void Sprite_SetRect(ExportSprite sprite, XMFLOAT2 pos, XMFLOAT2 size);
 