#include "Export.h"
#include "Utility.h"
#include "IRenderSystem.h"
#include "TRenderSystem11.h"
#include "TRenderSystem9.h"
#include "IRenderable.h"
#include "TMaterial.h"

IRenderSystem* RenderSystemCreate(HINSTANCE hInstance, HWND hWnd, bool isd3d11)
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

void RenderSystemDestroy(IRenderSystem* rendersys)
{
	rendersys->CleanUp();
	rendersys->Release();
}

void RenderSystemRender(IRenderSystem* rendersys, XMFLOAT4 bgColor, IRenderable** renderables, int renderableCount)
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
