#include "TInterfaceType.h"
#include "IRenderSystem.h"
#include "TInterfaceType11.h"

template<class T>
static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}
/********** TBlobDataStd **********/
TBlobDataStd::TBlobDataStd(const std::vector<char>& buffer)
	:mBuffer(buffer)
{
}

char* TBlobDataStd::GetBufferPointer()
{
	return mBuffer.empty() ? nullptr : &mBuffer[0];
}

size_t TBlobDataStd::GetBufferSize()
{
	return mBuffer.size();
}

/********** TProgram **********/
void TProgram::SetVertex(IVertexShaderPtr pVertex)
{
	assert(pVertex);
	mVertex = pVertex;
	AddDependency(pVertex);
}

void TProgram::SetPixel(IPixelShaderPtr pPixel)
{
	assert(pPixel);
	mPixel = pPixel;
	AddDependency(pPixel);
}

/********** ITexture **********/
ID3D11ShaderResourceView*& ITexture::GetSRV11()
{
	static ID3D11ShaderResourceView* srv;
	return srv;
}

IDirect3DTexture9*& ITexture::GetSRV9()
{
	static IDirect3DTexture9* srv;
	return srv;
}

IDirect3DCubeTexture9*& ITexture::GetSRVCube9()
{
	static IDirect3DCubeTexture9* srv;
	return srv;
}

bool ITexture::HasSRV()
{
	return GetSRV11() || GetSRV9() || GetSRVCube9();
}

/********** IHardwareBuffer **********/
ID3D11Buffer*& IHardwareBuffer::GetBuffer11()
{
	static ID3D11Buffer* buffer;
	return buffer;
}

unsigned int IHardwareBuffer::GetBufferSize()
{
	return 0;
}

/********** IVertexBuffer **********/
enHardwareBufferType IVertexBuffer::GetType()
{
	return E_HWBUFFER_VERTEX;
}

IDirect3DVertexBuffer9*& IVertexBuffer::GetBuffer9()
{
	static IDirect3DVertexBuffer9* buffer;
	return buffer;
}
#if 0
unsigned int IVertexBuffer::GetStride()
{
	return 0;
}
unsigned int IVertexBuffer::GetOffset()
{
	return 0;
}
#endif

/********** IIndexBuffer **********/
enHardwareBufferType IIndexBuffer::GetType()
{
	return E_HWBUFFER_INDEX;
}

IDirect3DIndexBuffer9*& IIndexBuffer::GetBuffer9()
{
	static IDirect3DIndexBuffer9* buffer;
	return buffer;
}

int IIndexBuffer::GetCount()
{
	return GetBufferSize() / GetWidth();
}

/********** IRenderTexture **********/
ID3D11RenderTargetView*& IRenderTexture::GetColorBuffer11()
{
	static ID3D11RenderTargetView* surface;
	return surface;
}

ID3D11DepthStencilView*& IRenderTexture::GetDepthStencilBuffer11()
{
	static ID3D11DepthStencilView* surface;
	return surface;
}

IDirect3DSurface9*& IRenderTexture::GetColorBuffer9()
{
	static IDirect3DSurface9* surface;
	return surface;
}

IDirect3DSurface9*& IRenderTexture::GetDepthStencilBuffer9()
{
	static IDirect3DSurface9* surface;
	return surface;
}

/********** IVertexShader **********/
ID3D11VertexShader*& IVertexShader::GetShader11()
{
	static ID3D11VertexShader* shader;
	return shader;
}

IDirect3DVertexShader9*& IVertexShader::GetShader9()
{
	static IDirect3DVertexShader9* shader;
	return shader;
}

/********** IPixelShader **********/
ID3D11PixelShader*& IPixelShader::GetShader11()
{
	static ID3D11PixelShader* shader;
	return shader;
}

IDirect3DPixelShader9*& IPixelShader::GetShader9()
{
	static IDirect3DPixelShader9* shader;
	return shader;
}

/********** IContantBuffer **********/
enHardwareBufferType IContantBuffer::GetType()
{
	return E_HWBUFFER_CONSTANT;
}

void* IContantBuffer::GetBuffer9()
{
	return nullptr;
}

/********** ISamplerState **********/
ID3D11SamplerState*& ISamplerState::GetSampler11()
{
	static ID3D11SamplerState* sampler;
	return sampler;
}

std::map<D3DSAMPLERSTATETYPE, DWORD>& ISamplerState::GetSampler9()
{
	static std::map<D3DSAMPLERSTATETYPE, DWORD> sampler;
	return sampler;
}
