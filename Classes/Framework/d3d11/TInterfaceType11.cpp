#include "TInterfaceType11.h"
#include "IRenderSystem.h"

template<class T>
static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}

/********** TTexture11 **********/
TTexture11::TTexture11(ID3D11ShaderResourceView* __texture, std::string __path)
{
	path = __path;
	texture = __texture;
}

void TTexture11::SetSRV11(ID3D11ShaderResourceView* __texture)
{
	texture = __texture;
}

IUnknown*& TTexture11::GetDeviceObject()
{
	return MakeDeviceObjectRef(texture);
}

const std::string& TTexture11::GetPath() const
{
	return path;
}

ID3D11ShaderResourceView*& TTexture11::GetSRV11()
{
	return texture;
}

D3D11_TEXTURE2D_DESC TTexture11::GetDesc()
{
	ID3D11Texture2D* pTexture;
	texture->GetResource((ID3D11Resource **)&pTexture);

	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);

	return desc;
}

int TTexture11::GetWidth()
{
	return GetDesc().Width;
}

int TTexture11::GetHeight()
{
	return GetDesc().Height;
}

DXGI_FORMAT TTexture11::GetFormat()
{
	return GetDesc().Format;
}

/********** TVertex11Buffer **********/
int TVertex11Buffer::GetCount()
{
	return bufferSize / stride;
}

ID3D11Buffer*& TVertex11Buffer::GetBuffer11()
{
	return buffer;
}

unsigned int TVertex11Buffer::GetBufferSize()
{
	return bufferSize;
}

unsigned int TVertex11Buffer::GetStride()
{
	return stride;
}

unsigned int TVertex11Buffer::GetOffset()
{
	return offset;
}