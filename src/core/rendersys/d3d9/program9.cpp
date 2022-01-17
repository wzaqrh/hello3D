#include "core/rendersys/d3d9/program9.h"
#include "core/base/base_type.h"
#include "core/base/debug.h"

namespace mir {

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
				ConstantTablePtr subTab = CreateInstance<ConstantTable>();
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

void ConstantTable::SetValue(IDirect3DDevice9* device, char* buffer9, const ConstBufferDecl& decl)
{
	for (auto& elem : decl) {
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

void ConstantTable::SetValue(IDirect3DDevice9* device, char* buffer9, const ConstBufferDeclElement& elem)
{
	D3DXHANDLE handle = GetHandle(elem.Name);
	if (handle) {
		HRESULT hr = S_OK;
		switch (elem.Type) {
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
		case kCBElementInt2:
		case kCBElementInt3:
		case kCBElementInt4:
		case kCBElementFloat:
		case kCBElementFloat2:
		case kCBElementFloat3:
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