#include "interface_type9.h"
#include "core/rendersys/resource.h"
#include "core/rendersys/render_system.h"
#include "core/base/utility.h"

namespace mir {

/********** TTexture9 **********/
Texture9::Texture9(IDirect3DTexture9 *__texture, const std::string& __path)
	: mTexture(__texture)
	, mTextureCube(nullptr)
	, mPath(__path)
{
	mRes = MakePtr<Resource>((IUnknown**)&mTexture);
	mRes->AddOnLoadedListener([this](IResource*) {
		D3DSURFACE_DESC desc = GetDesc();
		mWidth = desc.Width;
		mHeight = desc.Height;
		mFormat = D3dEnumConvert::d3d9To11(desc.Format);
		mMipCount = mTexture->GetLevelCount();
	});
}

Texture9::Texture9(int width, int height, DXGI_FORMAT format, int mipmap)
	: mTexture(nullptr)
	, mTextureCube(nullptr)
	, mPath()
{
	mWidth = width;
	mHeight = height;
	mFormat = format;
	mMipCount = mipmap;
	mRes = MakePtr<Resource>((IUnknown**)&mTexture);
}

void Texture9::SetSRV9(IDirect3DTexture9* __texture)
{
	mTexture = __texture;
}

const char* Texture9::GetPath() {
	return mPath.c_str();
}
int Texture9::GetWidth() {
	return mWidth;
}
int Texture9::GetHeight() {
	return mHeight;
}
DXGI_FORMAT Texture9::GetFormat() {
	return mFormat;// D3DEnumCT::d3d9To11(GetDesc().Format);
}
int Texture9::GetMipmapCount() {
	return mMipCount;
}
D3DSURFACE_DESC Texture9::GetDesc()
{
	D3DSURFACE_DESC desc;
	mTexture->GetLevelDesc(0, &desc);
	return desc;
}

/********** TIndexBuffer9 **********/
IDirect3DIndexBuffer9*& IndexBuffer9::GetBuffer9()
{
	return Buffer;
}

unsigned int IndexBuffer9::GetBufferSize()
{
	return BufferSize;
}

int IndexBuffer9::GetWidth()
{
	return D3dEnumConvert::GetWidth(Format);
}

DXGI_FORMAT IndexBuffer9::GetFormat()
{
	return Format;
}

/********** TVertexShader9 **********/
unsigned int VertexBuffer9::GetBufferSize()
{
	return BufferSize;
}

IDirect3DVertexBuffer9*& VertexBuffer9::GetBuffer9()
{
	return Buffer;
}

unsigned int VertexBuffer9::GetStride()
{
	return Stride;
}

unsigned int VertexBuffer9::GetOffset()
{
	return Offset;
}

/********** TRenderTexture9 **********/
RenderTexture9::RenderTexture9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer)
{
	mColorTexture = colorTexture;
	mColorTexture->AsRes()->SetLoaded();
	mColorBuffer = nullptr;
	mDepthStencilBuffer = depthStencilBuffer;
}

ITexturePtr RenderTexture9::GetColorTexture()
{
	return mColorTexture;
}

IDirect3DSurface9*& RenderTexture9::GetColorBuffer9()
{
	mColorBuffer = nullptr;
	if (CheckHR(mColorTexture->GetSRV9()->GetSurfaceLevel(0, &mColorBuffer))) return mColorBuffer;
	return mColorBuffer;
}

IDirect3DSurface9*& RenderTexture9::GetDepthStencilBuffer9()
{
	return mDepthStencilBuffer;
}

/********** TInputLayout9 **********/
InputLayout9::InputLayout9()
{
	mRes = MakePtr<Resource>((IUnknown**)&mLayout);
}

IDirect3DVertexDeclaration9*& InputLayout9::GetLayout9()
{
	return mLayout;
}

void ConstantTable::ReInit()
{
	mHandles.clear();
	mHandleByName.clear();
	Init(mTable);
}

/********** TConstantTable **********/
void ConstantTable::Init(ID3DXConstantTable* constTable, D3DXHANDLE handle, int constCount)
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
				ConstantTablePtr subTab = std::make_shared<ConstantTable>();
				subTab->Init(constTable, handleName.second, constDesc.StructMembers);
				mSubByName[handleName.first] = subTab;
			}
		}
		mHandles.push_back(handleName.second);
	}
}

ID3DXConstantTable* ConstantTable::get()
{
	return mTable;
}

size_t ConstantTable::size() const
{
	return mHandles.size();
}

D3DXHANDLE ConstantTable::GetHandle(size_t pos) const
{
	return mHandles[pos];
}

D3DXHANDLE ConstantTable::GetHandle(const std::string& name) const
{
	auto iter = mHandleByName.find(name);
	if (iter != mHandleByName.end()) {
		return iter->second;
	}
	else return NULL;
}

ConstantTablePtr ConstantTable::At(const std::string& name)
{
	auto iter = mSubByName.find(name);
	if (iter != mSubByName.end()) {
		return iter->second;
	}
	else return NULL;
}

void ConstantTable::SetValue(IDirect3DDevice9* device, char* buffer9, ConstBufferDecl& decl)
{
	for (size_t j = 0; j < decl.Elements.size(); ++j) {
		ConstBufferDeclElement& elem = decl.Elements[j];
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

void ConstantTable::SetValue(IDirect3DDevice9* device, char* buffer9, ConstBufferDeclElement& elem)
{
	D3DXHANDLE handle = GetHandle(elem.Name);
	if (handle) {
		HRESULT hr = S_OK;
		switch (elem.Type)
		{
		case kCBElementMatrix: {
			if (elem.Count == 0) {
				assert(elem.Size == sizeof(D3DXMATRIX));
				hr = mTable->SetMatrixTranspose(device, handle, (CONST D3DXMATRIX*)(buffer9 + elem.Offset));
			}
			else {
				assert(elem.Size == sizeof(D3DXMATRIX) * elem.Count);
				hr = mTable->SetMatrixTransposeArray(device, handle, (CONST D3DXMATRIX*)(buffer9 + elem.Offset), elem.Count);
			}
		}break;
		case kCBElementStruct:
		case kCBElementBool:
		case kCBElementInt:
		case kCBElementFloat:
		case kCBElementFloat4:
		default:
			hr = mTable->SetValue(device, handle, buffer9 + elem.Offset, elem.Size);
			break;
		}
		CheckHR(hr);
	}
}

/********** TPixelShader9 **********/
PixelShader9::PixelShader9()
{
	mRes = MakePtr<Resource>((IUnknown**)&mShader);
}

IBlobDataPtr PixelShader9::GetBlob()
{
	return mBlob;
}

IDirect3DPixelShader9*& PixelShader9::GetShader9()
{
	return mShader;
}

void PixelShader9::SetConstTable(ID3DXConstantTable* constTable)
{
	mConstTable.Init(constTable);
}

/********** TVertexShader9 **********/
VertexShader9::VertexShader9()
{
	mRes = MakePtr<Resource>((IUnknown**)&mShader);
}

IBlobDataPtr VertexShader9::GetBlob()
{
	return mBlob;
}

IDirect3DVertexShader9*& VertexShader9::GetShader9()
{
	return mShader;
}

void VertexShader9::SetConstTable(ID3DXConstantTable* constTable)
{
	mConstTable.Init(constTable);
}

/********** TBlobDataD3d9 **********/
BlobData9::BlobData9(ID3DXBuffer* pBlob)
	:mBlob(pBlob)
{
}

char* BlobData9::GetBufferPointer()
{
	return (char*)mBlob->GetBufferPointer();
}

size_t BlobData9::GetBufferSize()
{
	return mBlob->GetBufferSize();
}

/********** TSamplerState9 **********/
std::map<D3DSAMPLERSTATETYPE, DWORD>& SamplerState9::GetSampler9()
{
	return mStates;
}

/********** TContantBuffer9 **********/
ContantBuffer9::ContantBuffer9(ConstBufferDeclPtr decl)
	:mDecl(decl)
{
	assert(mDecl != nullptr);
	mBuffer9.resize(mDecl->BufferSize);
}

ConstBufferDeclPtr ContantBuffer9::GetDecl()
{
	return mDecl;
}

unsigned int ContantBuffer9::GetBufferSize()
{
	return mDecl->BufferSize;
}

char* ContantBuffer9::GetBuffer9()
{
	return mBuffer9.empty() ? nullptr : &mBuffer9[0];
}

void ContantBuffer9::SetBuffer9(char* data, int dataSize)
{
	mBuffer9.assign((char*)data, (char*)data + dataSize);
}

/********** TProgram9 **********/
Program9::Program9()
{
	mRes = MakePtr<Resource>((IUnknown**)0);
}

void Program9::SetVertex(VertexShader9Ptr pVertex) {
	mVertex = pVertex;
	mRes->AddDependency(pVertex->AsRes());
}
void Program9::SetPixel(PixelShader9Ptr pPixel) {
	mPixel = pPixel;
	mRes->AddDependency(pPixel->AsRes());
}

}