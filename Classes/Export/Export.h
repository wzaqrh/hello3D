#pragma once
#include "std.h"
#include "TPredefine.h"
#include "IRenderSystem.h"

IRenderSystem* RenderSystemCreate(HINSTANCE hInstance, HWND hWnd, bool isd3d11);
void RenderSystemDestroy(IRenderSystem* rendersys);
void RenderSystemRender(IRenderSystem* rendersys, XMFLOAT4 bgColor, IRenderable** renderables, int renderableCount);
