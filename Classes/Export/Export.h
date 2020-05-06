#pragma once
#include "std.h"
#include "TPredefine.h"
#include "IRenderSystem.h"
#include "TSprite.h"

#define DLL_EXPORT extern "C" __declspec(dllexport)

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
DLL_EXPORT ExportRenderSystem RenderSystem_Create(HWND hWnd, bool isd3d11);
DLL_EXPORT void RenderSystem_Destroy(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_Render(ExportRenderSystem rendersys, XMFLOAT4 bgColor, ExportRenderable* renderables, int renderableCount);

struct ExportTexture {
	ITexture* self;
	ExportTexture(ITexture* self__ = nullptr) :self(self__) {}
	ITexture* operator->() { return self; }
};
DLL_EXPORT ExportTexture Texture_GetByPath(ExportRenderSystem rendersys, const char* imgPath);

//TSprite
struct ExportSprite : ExportRenderable {
	ExportSprite(TSprite* self__ = nullptr) :ExportRenderable(self__) {}
	TSprite* operator->() { return static_cast<TSprite*>(self); }
	void DestroySelf() { delete self; }
};
DLL_EXPORT ExportSprite Sprite_Create(ExportRenderSystem rendersys, const char* imgPath);
DLL_EXPORT void Sprite_Destroy(ExportSprite sprite);
DLL_EXPORT void Sprite_SetTexture(ExportSprite sprite, ExportTexture texture);
DLL_EXPORT void Sprite_SetRect(ExportSprite sprite, XMFLOAT2 pos, XMFLOAT2 size);
#pragma pack(pop) 