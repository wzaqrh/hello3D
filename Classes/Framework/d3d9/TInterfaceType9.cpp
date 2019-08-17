#include "TInterfaceType9.h"
#include "IRenderSystem.h"
#include "Utility.h"

template<class T>
static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}
/********** TTexture9 **********/
TTexture9::TTexture9(const std::string& __path)
	:texture(nullptr)
	, textureCube(nullptr)
	, path(__path)
{
}

TTexture9::TTexture9(IDirect3DTexture9 *__texture, const std::string& __path)
	:texture(__texture)
	, textureCube(nullptr)
	, path(__path)
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

IDirect3DCubeTexture9*& TTexture9::GetSRVCube9()
{
	return textureCube;
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
TRenderTexture9::TRenderTexture9(TTexture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer)
{
	mColorTexture = colorTexture;
	mColorTexture->SetLoaded();
	mColorBuffer = nullptr;
	mDepthStencilBuffer = depthStencilBuffer;
}

ITexturePtr TRenderTexture9::GetColorTexture()
{
	return mColorTexture;
}

IDirect3DSurface9*& TRenderTexture9::GetColorBuffer9()
{
	mColorBuffer = nullptr;
	if (CheckHR(mColorTexture->GetSRV9()->GetSurfaceLevel(0, &mColorBuffer))) return mColorBuffer;
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

void TConstantTable::ReInit()
{
	mHandles.clear();
	mHandleByName.clear();
	Init(mTable);
}

/********** TConstantTable **********/
void TConstantTable::Init(ID3DXConstantTable* constTable, D3DXHANDLE handle, int constCount)
{
	mTable = constTable;

	if (constCount == 0) {
		D3DXCONSTANTTABLE_DESC desc;
		CheckHR(constTable->GetDesc(&desc));
		constCount = desc.Constants;
	}

	D3DXCONSTANT_DESC constDesc;
	std::pair<std::string, D3DXHANDLE> handleName;
	for (size_t reg = 0; reg < constCount; ++reg) {
		handleName.second = constTable->GetConstant(handle, reg);
		UINT count = 1;
		constTable->GetConstantDesc(handleName.second, &constDesc, &count);
		if (count >= 1) {
			handleName.first = constDesc.Name;
			mHandleByName.insert(handleName);

			if (constDesc.Class == D3DXPC_STRUCT) {
				TConstantTablePtr subTab = std::make_shared<TConstantTable>();
				subTab->Init(constTable, handleName.second, constDesc.StructMembers);
				mSubByName[handleName.first] = subTab;
			}
		}
		mHandles.push_back(handleName.second);
	}
}

ID3DXConstantTable* TConstantTable::get()
{
	return mTable;
}

size_t TConstantTable::size() const
{
	return mHandles.size();
}

D3DXHANDLE TConstantTable::GetHandle(size_t pos) const
{
	return mHandles[pos];
}

D3DXHANDLE TConstantTable::GetHandle(const std::string& name) const
{
	auto iter = mHandleByName.find(name);
	if (iter != mHandleByName.end()) {
		return iter->second;
	}
	else return NULL;
}

TConstantTablePtr TConstantTable::At(const std::string& name)
{
	auto iter = mSubByName.find(name);
	if (iter != mSubByName.end()) {
		return iter->second;
	}
	else return NULL;
}

void TConstantTable::SetValue(IDirect3DDevice9* device, char* buffer9, TConstBufferDecl& decl)
{
	for (size_t j = 0; j < decl.elements.size(); ++j) {
		TConstBufferDeclElement& elem = decl.elements[j];
		const char* pName = elem.name.c_str();
#if 0
		auto iter = decl.subDecls.find(elem.name);
		if (iter != decl.subDecls.end()) {
			TConstantTablePtr subTab = At(elem.name);
			if (subTab) {
				subTab->SetValue(device, buffer9 + elem.offset, iter->second);
			}
		}
		else 
#endif
			SetValue(device, buffer9, elem);
	}
}

void TConstantTable::SetValue(IDirect3DDevice9* device, char* buffer9, TConstBufferDeclElement& elem)
{
	D3DXHANDLE handle = GetHandle(elem.name);
	if (handle) {
		HRESULT hr = S_OK;
		switch (elem.type)
		{
		case E_CONSTBUF_ELEM_MATRIX: {
			if (elem.count == 0) {
				assert(elem.size == sizeof(D3DXMATRIX));
				hr = mTable->SetMatrixTranspose(device, handle, (CONST D3DXMATRIX*)(buffer9 + elem.offset));
			}
			else {
				assert(elem.size == sizeof(D3DXMATRIX) * elem.count);
				hr = mTable->SetMatrixTransposeArray(device, handle, (CONST D3DXMATRIX*)(buffer9 + elem.offset), elem.count);
			}
		}break;
		case E_CONSTBUF_ELEM_STRUCT:
		case E_CONSTBUF_ELEM_BOOL:
		case E_CONSTBUF_ELEM_INT:
		case E_CONSTBUF_ELEM_FLOAT:
		case E_CONSTBUF_ELEM_FLOAT4:
		default:
			hr = mTable->SetValue(device, handle, buffer9 + elem.offset, elem.size);
			break;
		}
		CheckHR(hr);
	}
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