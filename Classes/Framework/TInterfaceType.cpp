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

void* TBlobDataStd::GetBufferPointer()
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
	mVertex = pVertex;
	AddDependency(pVertex);
}

void TProgram::SetPixel(IPixelShaderPtr pPixel)
{
	mPixel = pPixel;
	AddDependency(pPixel);
}

/********** IInputLayout **********/
ID3D11InputLayout*& IInputLayout::GetLayout11()
{
	static ID3D11InputLayout* layout;
	return layout;
}

IDirect3DVertexDeclaration9*& IInputLayout::GetLayout9()
{
	static IDirect3DVertexDeclaration9* layout;
	return layout;
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
IDirect3DVertexBuffer9*& IVertexBuffer::GetBuffer9()
{
	static IDirect3DVertexBuffer9* buffer;
	return buffer;
}

unsigned int IVertexBuffer::GetStride()
{
	return 0;
}

unsigned int IVertexBuffer::GetOffset()
{
	return 0;
}

/********** IIndexBuffer **********/
IDirect3DIndexBuffer9*& IIndexBuffer::GetBuffer9()
{
	static IDirect3DIndexBuffer9* buffer;
	return buffer;
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

/********** ISamplerState **********/
ID3D11SamplerState*& ISamplerState::GetSampler11()
{
	static ID3D11SamplerState* sampler;
	return sampler;
}
