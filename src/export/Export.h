#pragma once
#include "core/base/std.h"
//INCLUDE_PREDEFINE_H
#include "core/rendersys/render_system.h"
#include "core/renderable/sprite.h"
#include "core/renderable/mesh.h"

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
typedef TMesh* ExportMesh;
typedef TCamera* ExportCamera;
typedef TTransform* ExportTransform;
#endif

//RenderSystem
DLL_EXPORT ExportRenderSystem RenderSystem_Create(HWND hWnd, bool isd3d11, RECT vp);
DLL_EXPORT void RenderSystem_Destroy(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_Render(ExportRenderSystem rendersys, XMFLOAT4 bgColor, ExportRenderable* renderables, int renderableCount);
DLL_EXPORT void RenderSystem_RClear(ExportRenderSystem rendersys, XMFLOAT4 bgColor);
DLL_EXPORT bool RenderSystem_RBeginScene(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_RRender(ExportRenderSystem rendersys, ExportRenderable* renderables, int renderableCount);
DLL_EXPORT void RenderSystem_REndScene(ExportRenderSystem rendersys);
DLL_EXPORT void RenderSystem_SetRenderTarget(ExportRenderSystem rendersys, ExportRenderTarget renderTarget);
DLL_EXPORT void RenderSystem_SetViewPort(ExportRenderSystem rendersys, int x, int y, int w, int h);
DLL_EXPORT XMINT4 RenderSystem_GetWinSize(ExportRenderSystem rendersys);

//Camera
DLL_EXPORT ExportCamera CameraOrtho_Create(ExportRenderSystem rendersys, int far1);
DLL_EXPORT ExportTransform Camera_GetTransform(ExportCamera camera);
DLL_EXPORT XMINT4 Camera_GetSize(ExportCamera camera);
DLL_EXPORT void Camera_SetFlipY(ExportCamera camera, bool flipY);

//Transform
DLL_EXPORT void Transform_SetScale(ExportTransform transform, XMFLOAT3 s);
DLL_EXPORT XMFLOAT3 Transform_GetScale(ExportTransform transform);
DLL_EXPORT void Transform_SetPosition(ExportTransform transform, XMFLOAT3 position);
DLL_EXPORT XMFLOAT3 Transform_GetPosition(ExportTransform transform);
DLL_EXPORT void Transform_SetEuler(ExportTransform transform, XMFLOAT3 euler);
DLL_EXPORT XMFLOAT3 Transform_GetEuler(ExportTransform transform);
DLL_EXPORT void Transform_SetFlipY(ExportTransform transform, bool flip);
DLL_EXPORT bool Transform_IsFlipY(ExportTransform transform);

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
 

//TMesh
DLL_EXPORT ExportMesh Mesh_Create(ExportRenderSystem rendersys, int vertCount, int indexCount);
DLL_EXPORT void Mesh_Clear(ExportMesh mesh);
DLL_EXPORT void Mesh_SetVertexs(ExportMesh mesh, const MeshVertex* vertData, int vertCount);
DLL_EXPORT void Mesh_SetVertexsPC(ExportMesh mesh, const MeshVertex* vertData, int vertCount, int vertPos);
DLL_EXPORT void Mesh_SetPositions(ExportMesh mesh, const XMFLOAT3* posData, int count);
DLL_EXPORT void Mesh_SetColors(ExportMesh mesh, const XMFLOAT4* colorData, int count);
DLL_EXPORT void Mesh_SetUVs(ExportMesh mesh, const XMFLOAT2* uvData, int count);
DLL_EXPORT void Mesh_SetSubMeshCount(ExportMesh mesh, int count);
DLL_EXPORT void Mesh_SetIndices(ExportMesh mesh, const UINT* indiceData, int indicePos, int indiceCount, int indiceBase, int subMeshIndex);
DLL_EXPORT void Mesh_SetTexture(ExportMesh mesh, int slot, ExportTexture texture, int subMeshIndex);