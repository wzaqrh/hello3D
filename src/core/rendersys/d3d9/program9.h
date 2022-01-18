#pragma once
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "core/rendersys/d3d9/predeclare.h"
#include "core/rendersys/program.h"

namespace mir {

class ConstantTable
{
public:
	ConstantTable();
	void ReInit();
	void Init(ID3DXConstantTable* constTable, D3DXHANDLE handle = NULL, int count = 0);
	ID3DXConstantTable* Get() { return mTable; }
	size_t Count() const { return mHandles.size(); }
	D3DXHANDLE GetHandle(size_t pos) const;
	D3DXHANDLE GetHandle(const std::string& name) const;
	std::shared_ptr<ConstantTable> At(const std::string& name);
public:
	void SetValue(IDirect3DDevice9* device, char* buffer9, const CbDeclElement& elem);
	void SetValue(IDirect3DDevice9* device, char* buffer9, const ConstBufferDecl& decl);
public:
	ID3DXConstantTable* mTable;
	std::vector<D3DXHANDLE> mHandles;
	std::map<std::string, D3DXHANDLE> mHandleByName;
	std::map<std::string, std::shared_ptr<ConstantTable>> mSubByName;
};

class VertexShader9 : public ImplementResource<IVertexShader>
{
public:
	VertexShader9();
	IBlobDataPtr GetBlob() const override { return mBlob; }
	IDirect3DVertexShader9*& GetShader9() { return mShader; }
	void SetConstTable(ID3DXConstantTable* constTable);
public:
	ConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DVertexShader9* mShader;
};

class PixelShader9 : public ImplementResource<IPixelShader>
{
public:
	PixelShader9();
	IBlobDataPtr GetBlob() const override { return mBlob; }
	IDirect3DPixelShader9*& GetShader9() { return mShader; }
	void SetConstTable(ID3DXConstantTable* constTable);
public:
	ConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DPixelShader9* mShader;
};

class Program9 : public ImplementResource<IProgram>
{
public:
	Program9();
	void SetVertex(VertexShader9Ptr pVertex);
	void SetPixel(PixelShader9Ptr pPixel);
	IVertexShaderPtr GetVertex() const override { return mVertex; }
	IPixelShaderPtr GetPixel() const override { return mPixel; }
public:
	VertexShader9Ptr mVertex;
	PixelShader9Ptr mPixel;
};

}