#include "Export.h"
#include "Utility.h"
#include "IRenderSystem.h"
#include "TRenderSystem11.h"
#include "TRenderSystem9.h"
#include "IRenderable.h"
#include "TMaterial.h"
#include "TSprite.h"

//RenderSystem
IRenderSystem* RenderSystem_Create(HINSTANCE hInstance, HWND hWnd, bool isd3d11)
{
	IRenderSystem* rendersys = nullptr;
	if (isd3d11) {
		rendersys = new TRenderSystem11;
	}
	else {
		rendersys = new TRenderSystem9;
	}

	rendersys->SetHandle(hInstance, hWnd);

	if (FAILED(rendersys->Initialize())) {
		rendersys->CleanUp();
		rendersys = nullptr;
	}

	return rendersys;
}

void RenderSystem_Destroy(IRenderSystem* rendersys)
{
	rendersys->CleanUp();
	rendersys->Release();
}

void RenderSystem_Render(IRenderSystem* rendersys, XMFLOAT4 bgColor, IRenderable** renderables, int renderableCount)
{
	rendersys->ClearColorDepthStencil(bgColor, 1.0f, 0);
	rendersys->Update(0);

	if (rendersys->BeginScene()) {
		TRenderOperationQueue opQueue;
		for (int i = 0; i < renderableCount; ++i) {
			renderables[i]->GenRenderOperation(opQueue);
			rendersys->RenderQueue(opQueue, E_PASS_FORWARDBASE);
		}
		rendersys->EndScene();
	}
}

//ITexture
ITexture* Texture_GetByPath(IRenderSystem* rendersys, const char* imgPath)
{
	ITexturePtr texture = rendersys->GetTexByPath(imgPath);
	return texture ? texture.Detach() : nullptr;
}

//TSprite
TSprite* Sprite_Create(IRenderSystem* rendersys, const char* imgPath)
{
	return new TSprite(rendersys, E_MAT_SPRITE);
}

void Sprite_Destroy(TSprite* sprite)
{
	delete sprite;
}

void Sprite_SetTexture(TSprite* sprite, ITexture* texture)
{
	sprite->SetTexture(ITexturePtr(texture));
}

void Sprite_SetRect(TSprite* sprite, XMFLOAT2 pos, XMFLOAT2 size)
{
	sprite->SetPosition(pos.x, pos.y, 0);
	sprite->SetSize(size.x, size.y);
}
