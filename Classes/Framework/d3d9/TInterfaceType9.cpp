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
