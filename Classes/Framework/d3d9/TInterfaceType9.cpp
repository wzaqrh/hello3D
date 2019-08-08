#include "TInterfaceType9.h"
#include "IRenderSystem.h"
#include "Utility.h"

template<class T>
static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}
/********** TTexture9 **********/
TTexture9::TTexture9(IDirect3DTexture9* __texture, std::string __path)
	:texture(__texture)
	,path(__path)
{
}
IUnknown*& TTexture9::GetDeviceObject()
{
	return MakeDeviceObjectRef(texture);
}
void TTexture9::SetSRV9(IDirect3DTexture9* __texture)
{
	texture = __texture;
}
IDirect3DTexture9*& TTexture9::GetSRV9()
{
	return texture;
}
const std::string& TTexture9::GetPath() const
{
	return path;
}
int TTexture9::GetWidth()
{
	return GetDesc().Width;
}
int TTexture9::GetHeight()
{
	return GetDesc().Height;
}
DXGI_FORMAT TTexture9::GetFormat()
{
	return D3DEnumCT::d3d9To11(GetDesc().Format);
}
D3DSURFACE_DESC TTexture9::GetDesc()
{
	D3DSURFACE_DESC desc;
	texture->GetLevelDesc(0, &desc);
	return desc;
}

/********** TIndexBuffer9 **********/
IDirect3DIndexBuffer9*& TIndexBuffer9::GetBuffer9()
{
	return buffer;
}

unsigned int TIndexBuffer9::GetBufferSize()
{
	return bufferSize;
}

int TIndexBuffer9::GetWidth()
{
	return D3DEnumCT::GetWidth(format);
}

DXGI_FORMAT TIndexBuffer9::GetFormat()
{
	return format;
}

/********** TVertexShader9 **********/
unsigned int TVertexBuffer9::GetBufferSize()
{
	return bufferSize;
}

IDirect3DVertexBuffer9*& TVertexBuffer9::GetBuffer9()
{
	return buffer;
}

unsigned int TVertexBuffer9::GetStride()
{
	return stride;
}

unsigned int TVertexBuffer9::GetOffset()
{
	return offset;
}

/********** TRenderTexture9 **********/
TRenderTexture9::TRenderTexture9(IDirect3DSurface9* colorBuffer, IDirect3DSurface9* depthStencilBuffer)
{
	mColorBuffer = colorBuffer;
	mDepthStencilBuffer = depthStencilBuffer;

	void *pContainer = NULL;
	HRESULT hr = mColorBuffer->GetContainer(IID_IDirect3DTexture9, &pContainer);
	if (SUCCEEDED(hr) && pContainer) {
		mColorTexture = std::make_shared<TTexture9>((IDirect3DTexture9 *)pContainer, "");
	}
}

ITexturePtr TRenderTexture9::GetColorTexture()
{
	return mColorTexture;
}

IDirect3DSurface9*& TRenderTexture9::GetColorBuffer9()
{
	return mColorBuffer;
}

IDirect3DSurface9*& TRenderTexture9::GetDepthStencilBuffer9()
{
	return mDepthStencilBuffer;
}
