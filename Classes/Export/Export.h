#pragma once
#include "std.h"
#include "TPredefine.h"
#include "IRenderSystem.h"

//IRenderSystem
IRenderSystem* RenderSystem_Create(HINSTANCE hInstance, HWND hWnd, bool isd3d11);
void RenderSystem_Destroy(IRenderSystem* rendersys);
void RenderSystem_Render(IRenderSystem* rendersys, XMFLOAT4 bgColor, IRenderable** renderables, int renderableCount);

//ITexture
ITexture* Texture_GetByPath(IRenderSystem* rendersys, const char* imgPath);

//TSprite
TSprite* Sprite_Create(IRenderSystem* rendersys, const char* imgPath);
void Sprite_Destroy(TSprite* sprite);
void Sprite_SetTexture(TSprite* sprite, ITexture* texture);
void Sprite_SetRect(TSprite* sprite, XMFLOAT2 pos, XMFLOAT2 size);
