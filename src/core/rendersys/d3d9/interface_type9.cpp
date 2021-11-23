#include "interface_type9.h"
#include "core/resource/resource.h"
#include "core/rendersys/render_system.h"
#include "core/base/debug.h"
#include "core/base/d3d.h"

namespace mir {

#define MakePtr std::make_shared
#define PtrRaw(T) T.get()

/********** Texture9 **********/
Texture9::Texture9(IDirect3DTexture9 *texture)
	: mTexture(texture)
	, mTextureCube(nullptr)
{
	mWidth = 0, mHeight = 0, mMipCount = 0;
	mFormat = kFormatUnknown;

	/*SetDeviceObject((IUnknown**)&mTexture);
	AsRes(this)->AddOnLoadedListener([this](IResource*) {
		D3DSURFACE_DESC desc = GetDesc();
		mWidth = desc.Width;
		mHeight = desc.Height;
		mFormat = static_cast<ResourceFormat>(d3d::convert9To11(desc.Format));
		mMipCount = mTexture->GetLevelCount();
	});*/
}

Texture9::Texture9(int width, int height, ResourceFormat format, int mipmap)
	: mTexture(nullptr)
	, mTextureCube(nullptr)
{
	mWidth = width;
	mHeight = height;
	mMipCount = mipmap;
	mFormat = format;

	//SetDeviceObject((IUnknown**)&mTexture);
}

void Texture9::SetSRV9(IDirect3DTexture9* texture)
{
	mTexture = texture;
}

D3DSURFACE_DESC Texture9::GetDesc()
{
	D3DSURFACE_DESC desc;
	mTexture->GetLevelDesc(0, &desc);
	return desc;
}

/********** TIndexBuffer9 **********/
int IndexBuffer9::GetWidth()
{
	return d3d::BytePerPixel(static_cast<DXGI_FORMAT>(Format));
}

/********** TRenderTexture9 **********/
RenderTexture9::RenderTexture9()
{
	mColorTexture = nullptr;
	mColorBuffer = nullptr;
	mDepthStencilBuffer = nullptr;
}
RenderTexture9::RenderTexture9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer)
{
	mColorTexture = colorTexture;
	AsRes(mColorTexture)->SetLoaded();
	mColorBuffer = nullptr;
	mDepthStencilBuffer = depthStencilBuffer;
}

IDirect3DSurface9*& RenderTexture9::GetColorBuffer9()
{
	mColorBuffer = nullptr;
	if (CheckHR(mColorTexture->GetSRV9()->GetSurfaceLevel(0, &mColorBuffer))) return mColorBuffer;
	return mColorBuffer;
}

/********** TInputLayout9 **********/
InputLayout9::InputLayout9()
{
	mLayout = nullptr;
	//SetDeviceObject((IUnknown**)&mLayout);
}

/********** TConstantTable **********/
ConstantTable::ConstantTable()
{
	mTable = nullptr;
}

void ConstantTable::ReInit()
{
	mHandles.clear();
	mHandleByName.clear();
	Init(mTable);
}

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
	mShader = nullptr;
	//SetDeviceObject((IUnknown**)&mShader);
}

void PixelShader9::SetConstTable(ID3DXConstantTable* constTable)
{
	mConstTable.Init(constTable);
}

/********** TVertexShader9 **********/
VertexShader9::VertexShader9()
{
	mShader = nullptr;
	//SetDeviceObject((IUnknown**)&mShader);
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
	//SetDeviceObject((IUnknown**)0);
}

void Program9::SetVertex(VertexShader9Ptr pVertex) {
	mVertex = pVertex;
	//AsRes(this)->AddDependency(AsRes(pVertex));
}
void Program9::SetPixel(PixelShader9Ptr pPixel) {
	mPixel = pPixel;
	//AsRes(this)->AddDependency(AsRes(pPixel));
}

}