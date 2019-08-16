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

/********** TInputLayout9 **********/
IDirect3DVertexDeclaration9*& TInputLayout9::GetLayout9()
{
	return mLayout;
}

/********** TConstantTable **********/
void TConstantTable::Init(ID3DXConstantTable* constTable)
{
	mTable = constTable;

	D3DXCONSTANTTABLE_DESC desc;
	CheckHR(constTable->GetDesc(&desc));

	D3DXCONSTANT_DESC constDesc;
	std::pair<std::string, D3DXHANDLE> handleName;
	for (size_t reg = 0; reg < desc.Constants; ++reg) {
		handleName.second = constTable->GetConstant(NULL, reg);
		UINT count = 1;
		constTable->GetConstantDesc(handleName.second, &constDesc, &count);
		if (count >= 1) {
			handleName.first = constDesc.Name;
			mHandleByName.insert(handleName);
		}
		mHandles.push_back(handleName.second);
	}
}

ID3DXConstantTable* TConstantTable::get()
{
	return mTable;
}

ID3DXConstantTable* TConstantTable::operator->()
{
	return mTable;
}

size_t TConstantTable::size() const
{
	return mHandles.size();
}

D3DXHANDLE TConstantTable::operator[](size_t pos) const
{
	return mHandles[pos];
}

D3DXHANDLE TConstantTable::operator[](const std::string& name) const
{
	auto iter = mHandleByName.find(name);
	if (iter != mHandleByName.end()) {
		return iter->second;
	}
	else
		return NULL;
}

/********** TPixelShader9 **********/
IBlobDataPtr TPixelShader9::GetBlob()
{
	return mBlob;
}

IDirect3DPixelShader9*& TPixelShader9::GetShader9()
{
	return mShader;
}

void TPixelShader9::SetConstTable(ID3DXConstantTable* constTable)
{
	mConstTable.Init(constTable);
}

/********** TVertexShader9 **********/
IBlobDataPtr TVertexShader9::GetBlob()
{
	return mBlob;
}

IDirect3DVertexShader9*& TVertexShader9::GetShader9()
{
	return mShader;
}

void TVertexShader9::SetConstTable(ID3DXConstantTable* constTable)
{
	mConstTable.Init(constTable);
}

/********** TBlobDataD3d9 **********/
TBlobDataD3d9::TBlobDataD3d9(ID3DXBuffer* pBlob)
	:mBlob(pBlob)
{
}

void* TBlobDataD3d9::GetBufferPointer()
{
	return mBlob->GetBufferPointer();
}

size_t TBlobDataD3d9::GetBufferSize()
{
	return mBlob->GetBufferSize();
}

/********** TSamplerState9 **********/
std::map<D3DSAMPLERSTATETYPE, DWORD>& TSamplerState9::GetSampler9()
{
	return mStates;
}

/********** TContantBuffer9 **********/
TContantBuffer9::TContantBuffer9(TConstBufferDeclPtr decl)
	:mDecl(decl)
{
	mBuffer9.resize(mDecl->bufferSize);
}

TConstBufferDeclPtr TContantBuffer9::GetDecl()
{
	return mDecl;
}

unsigned int TContantBuffer9::GetBufferSize()
{
	return mDecl->bufferSize;
}

void* TContantBuffer9::GetBuffer9()
{
	return mBuffer9.empty() ? nullptr : &mBuffer9[0];
}