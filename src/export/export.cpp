#include "export/export.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/d3d11/render_system11.h"
#include "core/rendersys/d3d9/render_system9.h"
#include "core/rendersys/scene_manager.h"
#include "core/renderable/renderable.h"
#include "core/rendersys/material_factory.h"
#include "core/renderable/sprite.h"
#include "core/base/transform.h"
#include "core/base/utility.h"

//RenderSystem
ExportRenderSystem RenderSystem_Create(HWND hWnd, bool isd3d11, RECT vp)
{
	ExportRenderSystem rendersys = nullptr;
	if (isd3d11) {
		rendersys = new mir::TRenderSystem11;
	}
	else {
		rendersys = new mir::TRenderSystem9;
	}

	if (FAILED(rendersys->Initialize(hWnd, vp))) {
		rendersys->CleanUp();
		rendersys = nullptr;
	}

	rendersys->GetSceneManager()->SetOthogonalCamera(100);

	rendersys->GetSceneManager()->AddPointLight();
	rendersys->SetDepthState(mir::TDepthState::For2D);
	rendersys->SetBlendFunc(mir::TBlendFunc::ALPHA_NON_PREMULTIPLIED);

	return rendersys;
}

void RenderSystem_Destroy(ExportRenderSystem rendersys)
{
	rendersys->CleanUp();
	delete rendersys;
}

void RenderSystem_Render(ExportRenderSystem rendersys, XMFLOAT4 bgColor, ExportRenderable* renderables, int renderableCount)
{
	RenderSystem_RClear(rendersys, bgColor);

	if (RenderSystem_RBeginScene(rendersys)) {
		RenderSystem_RRender(rendersys, renderables, renderableCount);
		RenderSystem_REndScene(rendersys);
	}
}
void RenderSystem_RClear(ExportRenderSystem rendersys, XMFLOAT4 bgColor)
{
	rendersys->ClearColorDepthStencil(bgColor, 1.0f, 0);
}
bool RenderSystem_RBeginScene(ExportRenderSystem rendersys)
{
	rendersys->Update(0);
	return rendersys->BeginScene();
}
void RenderSystem_RRender(ExportRenderSystem rendersys, ExportRenderable* renderables, int renderableCount)
{
	mir::TRenderOperationQueue opQueue;
	for (int i = 0; i < renderableCount; ++i) {
		renderables[i]->GenRenderOperation(opQueue);
		rendersys->RenderQueue(opQueue, E_PASS_FORWARDBASE);
	}
}
void RenderSystem_REndScene(ExportRenderSystem rendersys)
{
	rendersys->EndScene();
}

DLL_EXPORT void RenderSystem_SetRenderTarget(ExportRenderSystem rendersys, ExportRenderTarget renderTarget)
{
	rendersys->SetRenderTarget(renderTarget ? MakePtr(renderTarget) : nullptr);
}

DLL_EXPORT void RenderSystem_SetViewPort(ExportRenderSystem rendersys, int x, int y, int w, int h)
{
	rendersys->SetViewPort(x, y, w, h);
}

DLL_EXPORT XMINT4 RenderSystem_GetWinSize(ExportRenderSystem rendersys)
{
	return rendersys->GetWinSize();
}

//camera
ExportCamera CameraOrtho_Create(ExportRenderSystem rendersys, int far1)
{
	mir::TCameraPtr camera = rendersys->GetSceneManager()->SetOthogonalCamera(far1);
	return camera.get();
}

ExportTransform Camera_GetTransform(ExportCamera camera)
{
	return camera->GetTransform().get();
}

DLL_EXPORT XMINT4 Camera_GetSize(ExportCamera camera)
{
	return {camera->GetWidth(), camera->GetHeight(), 0, 0};
}

DLL_EXPORT void Camera_SetFlipY(ExportCamera camera, bool flipY)
{
	camera->SetFlipY(flipY);
}

//Transform
void Transform_SetScale(ExportTransform transform, XMFLOAT3 s)
{
	transform->SetScale(s);
}

DLL_EXPORT XMFLOAT3 Transform_GetScale(ExportTransform transform)
{
	return transform->GetScale();
}

void Transform_SetPosition(ExportTransform transform, XMFLOAT3 position)
{
	transform->SetPosition(position);
}

DLL_EXPORT XMFLOAT3 Transform_GetPosition(ExportTransform transform)
{
	return transform->GetPosition();
}

void Transform_SetEuler(ExportTransform transform, XMFLOAT3 euler)
{
	transform->SetEuler(euler);
}

DLL_EXPORT XMFLOAT3 Transform_GetEuler(ExportTransform transform)
{
	return transform->GetEuler();
}

DLL_EXPORT void Transform_SetFlipY(ExportTransform transform, bool flip)
{
	transform->SetFlipY(flip);
}

DLL_EXPORT bool Transform_IsFlipY(ExportTransform transform)
{
	return transform->IsFlipY();
}

#ifdef USE_EXPORT_COM
template<class T> T* GetExportPtr(const ComPtr<T>& ptr) {
	return ptr ? ptr.Detach() : nullptr;
}
#else
template<class T> T* GetExportPtr(std::shared_ptr<T>& ptr) {
	T* texture = nullptr;
	if (ptr) {
		texture = PtrRaw(ptr);
		if (texture) texture->AddRef();
		ptr.reset();
	}
	return texture;
}
#endif

//RenderTarget
DLL_EXPORT ExportRenderTarget RenderTarget_Create(ExportRenderSystem rendersys, int width, int height, DXGI_FORMAT format)
{
	mir::IRenderTexturePtr rt = rendersys->CreateRenderTexture(width, height, format);
	return GetExportPtr(rt);
}

DLL_EXPORT ExportTexture RenderTarget_GetTexture(ExportRenderTarget renderTarget)
{
	mir::ITexturePtr texture = renderTarget ? renderTarget->GetColorTexture() : nullptr;
	return GetExportPtr(texture);
}

//ITexture
DLL_EXPORT ExportTexture Texture_Load(ExportRenderSystem rendersys, const char* imgPath, bool async)
{
	mir::ITexturePtr texture = rendersys->LoadTexture(imgPath, DXGI_FORMAT_UNKNOWN, async);
	return GetExportPtr(texture);
}

DLL_EXPORT ExportTexture Texture_Create(ExportRenderSystem rendersys, int width, int height, DXGI_FORMAT format, int mipmap)
{
	mir::ITexturePtr texture = rendersys->CreateTexture(width, height, format, mipmap);
	return GetExportPtr(texture);
}

DLL_EXPORT bool Texture_LoadRawData(ExportRenderSystem rendersys, ExportTexture texture, PBYTE data, int dataSize, int dataStep)
{
	return rendersys->LoadRawTextureData(MakePtr(texture), (char*)data, dataSize, dataStep);
}

DLL_EXPORT int Texture_Width(ExportTexture texture)
{
	return texture->GetWidth();
}

DLL_EXPORT int Texture_Height(ExportTexture texture)
{
	return texture->GetHeight();
}

DLL_EXPORT DXGI_FORMAT Texture_Format(ExportTexture texture)
{
	return texture->GetFormat();
}

DLL_EXPORT int Texture_MipmapCount(ExportTexture texture)
{
	return texture->GetMipmapCount();
}

//TSprite
DLL_EXPORT ExportSprite SpriteColor_Create(ExportRenderSystem rendersys, XMFLOAT4 color)
{
#ifdef EXPORT_STRUCT
	TSprite* sprite = new TSprite(rendersys.self, E_MAT_LAYERCOLOR);
#else
	mir::TSprite* sprite = new mir::TSprite(rendersys, E_MAT_LAYERCOLOR);
#endif
	sprite->SetColor(color);
	return sprite;
}

ExportSprite SpriteImage_Create(ExportRenderSystem rendersys, const char* imgPath)
{
#ifdef EXPORT_STRUCT
	TSprite* sprite = new TSprite(rendersys.self, E_MAT_SPRITE);
#else
	mir::TSprite* sprite = new mir::TSprite(rendersys, E_MAT_SPRITE);
#endif
	if (imgPath != nullptr && imgPath != "")
		sprite->SetTexture(rendersys->LoadTexture(imgPath));
	return sprite;
}

void Sprite_Destroy(ExportSprite sprite)
{
#ifdef EXPORT_STRUCT
	sprite.DestroySelf();
#else
	delete sprite;
#endif
}

void Sprite_SetTexture(ExportSprite sprite, ExportTexture texture)
{
#ifdef EXPORT_STRUCT
	sprite->SetTexture(ITexturePtr(texture.self));
#else
	sprite->SetTexture(MakePtr(texture));
#endif
}

void Sprite_SetRect(ExportSprite sprite, XMFLOAT2 pos, XMFLOAT2 size)
{
	sprite->SetPosition(pos.x, pos.y, 0);
	sprite->SetSize(size.x, size.y);
}

DLL_EXPORT void Sprite_SetColor(ExportSprite sprite, XMFLOAT4 color)
{
	sprite->SetColor(color);
}

DLL_EXPORT void Sprite_SetFlipY(ExportSprite sprite, bool flipY)
{
	sprite->SetFlipY(flipY);
}

//Mesh
DLL_EXPORT ExportMesh Mesh_Create(ExportRenderSystem rendersys, int vertCount, int indexCount)
{
	return new mir::TMesh(rendersys, E_MAT_SPRITE, vertCount, indexCount);
}

DLL_EXPORT void Mesh_Clear(ExportMesh mesh)
{
	mesh->Clear();
}

DLL_EXPORT void Mesh_SetVertexs(ExportMesh mesh, const ExportMeshVertex* vertData, int vertCount)
{
	mesh->SetVertexs(vertData, vertCount);
}

DLL_EXPORT void Mesh_SetVertexsPC(ExportMesh mesh, const ExportMeshVertex* vertData, int vertCount, int vertPos)
{
	mesh->SetVertexs(vertData, vertCount, vertPos);
}

DLL_EXPORT void Mesh_SetPositions(ExportMesh mesh, const XMFLOAT3* posData, int count)
{
	mesh->SetPositions(posData, count);
}

DLL_EXPORT void Mesh_SetColors(ExportMesh mesh, const XMFLOAT4* colorData, int count)
{
	mesh->SetColors(colorData, count);
}

DLL_EXPORT void Mesh_SetUVs(ExportMesh mesh, const XMFLOAT2* uvData, int count)
{
	mesh->SetUVs(uvData, count);
}

DLL_EXPORT void Mesh_SetSubMeshCount(ExportMesh mesh, int count)
{
	mesh->SetSubMeshCount(count);
}

DLL_EXPORT void Mesh_SetIndices(ExportMesh mesh, const UINT* indiceData, int indicePos, int indiceCount, int indiceBase, int subMeshIndex)
{
	mesh->SetIndices(indiceData, indicePos, indiceCount, indiceBase, subMeshIndex);
}

DLL_EXPORT void Mesh_SetTexture(ExportMesh mesh, int slot, ExportTexture texture, int subMeshIndex)
{
	mesh->SetTexture(slot, MakePtr(texture), subMeshIndex);
}