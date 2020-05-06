#include "Export.h"
#include "Utility.h"
#include "IRenderSystem.h"
#include "TRenderSystem11.h"
#include "TRenderSystem9.h"
#include "IRenderable.h"
#include "TMaterial.h"
#include "TSprite.h"

//RenderSystem
ExportRenderSystem RenderSystem_Create(HWND hWnd, bool isd3d11)
{
	ExportRenderSystem rendersys = nullptr;
	if (isd3d11) {
		rendersys = new TRenderSystem11;
	}
	else {
		rendersys = new TRenderSystem9;
	}

	if (FAILED(rendersys->Initialize(hWnd))) {
		rendersys->CleanUp();
		rendersys = nullptr;
	}

	rendersys->SetOthogonalCamera(100);
	rendersys->AddPointLight();

	return rendersys;
}

void RenderSystem_Destroy(ExportRenderSystem rendersys)
{
	rendersys->CleanUp();
	rendersys->Release();
}

void RenderSystem_Render(ExportRenderSystem rendersys, XMFLOAT4 bgColor, ExportRenderable* renderables, int renderableCount)
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
ExportTexture Texture_GetByPath(ExportRenderSystem rendersys, const char* imgPath)
{
	ITexturePtr texture = rendersys->GetTexByPath(imgPath);
	return texture ? texture.Detach() : nullptr;
}

//TSprite
ExportSprite Sprite_Create(ExportRenderSystem rendersys, const char* imgPath)
{
	TSprite* sprite = new TSprite(rendersys.self, E_MAT_SPRITE);
	if (imgPath != nullptr && imgPath != "")
		sprite->SetTexture(rendersys->GetTexByPath(imgPath));
	return ExportSprite(sprite);
}

void Sprite_Destroy(ExportSprite sprite)
{
	sprite.DestroySelf();
}

void Sprite_SetTexture(ExportSprite sprite, ExportTexture texture)
{
	sprite->SetTexture(ITexturePtr(texture.self));
}

void Sprite_SetRect(ExportSprite sprite, XMFLOAT2 pos, XMFLOAT2 size)
{
	sprite->SetPosition(pos.x, pos.y, 0);
	sprite->SetSize(size.x, size.y);
}
