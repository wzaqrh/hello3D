#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "core/rendersys/interface_type.h"

namespace mir {

typedef std::shared_ptr<struct BlobData9> BlobData9Ptr;
typedef std::shared_ptr<struct InputLayout9> InputLayout9Ptr;
typedef std::shared_ptr<struct ConstantTable> ConstantTablePtr;
typedef std::shared_ptr<struct VertexShader9> VertexShader9Ptr;
typedef std::shared_ptr<struct PixelShader9> PixelShader9Ptr;
typedef std::shared_ptr<struct Program9> Program9Ptr;
typedef std::shared_ptr<struct IndexBuffer9> IndexBuffer9Ptr;
typedef std::shared_ptr<struct VertexBuffer9> VertexBuffer9Ptr;
typedef std::shared_ptr<struct ContantBuffer9> ContantBuffer9Ptr;
typedef std::shared_ptr<struct Texture9> Texture9Ptr;
typedef std::shared_ptr<struct RenderTexture9> RenderTexture9Ptr;
typedef std::shared_ptr<struct SamplerState9> SamplerState9Ptr;

class BlobData9 : public IBlobData 
{
public:
	BlobData9(ID3DXBuffer* pBlob);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
public:
	ID3DXBuffer* mBlob;
};

class InputLayout9 : public ImplementResource<IInputLayout> 
{
public:
	InputLayout9();
	//IResourcePtr AsRes() override { return mRes; }

	IDirect3DVertexDeclaration9*& GetLayout9() { return mLayout; }
public:
	std::vector<D3DVERTEXELEMENT9> mInputDescs;
	IDirect3DVertexDeclaration9* mLayout;
	//IResourcePtr mRes;
};

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
	void SetValue(IDirect3DDevice9* device, char* buffer9, ConstBufferDeclElement& elem);
	void SetValue(IDirect3DDevice9* device, char* buffer9, ConstBufferDecl& decl);
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
	//IResourcePtr AsRes() override { return mRes; }

	IBlobDataPtr GetBlob() override { return mBlob; }
	IDirect3DVertexShader9*& GetShader9() { return mShader; }
	void SetConstTable(ID3DXConstantTable* constTable);
public:
	ConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DVertexShader9* mShader;
	//IResourcePtr mRes;
};

class PixelShader9 : public ImplementResource<IPixelShader> 
{
public:
	PixelShader9();
	//IResourcePtr AsRes() override { return mRes; }

	virtual IBlobDataPtr GetBlob() override { return mBlob; }
	IDirect3DPixelShader9*& GetShader9() { return mShader; }
	void SetConstTable(ID3DXConstantTable* constTable);
public:
	ConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DPixelShader9* mShader;
	//IResourcePtr mRes;
};

class Program9 : public ImplementResource<IProgram> 
{
public:
	Program9();
	//IResourcePtr AsRes() override { return mRes; }
	
	void SetVertex(VertexShader9Ptr pVertex);
	void SetPixel(PixelShader9Ptr pPixel);
	IVertexShaderPtr GetVertex() override { return mVertex; }
	IPixelShaderPtr GetPixel() override { return mPixel; }
public:
	VertexShader9Ptr mVertex;
	PixelShader9Ptr mPixel;
	//IResourcePtr mRes;
};

/********** HardwareBuffer **********/
class IndexBuffer9 : public ImplementResource<IIndexBuffer> 
{
public:
	IndexBuffer9(): Buffer(nullptr), BufferSize(0), Format(kFormatUnknown) {}
	void Init(IDirect3DIndexBuffer9* buffer, unsigned int bufferSize, ResourceFormat format) {
		Buffer = buffer; 
		BufferSize = bufferSize; 
		Format = format;
	}

	IDirect3DIndexBuffer9*& GetBuffer9() { return Buffer; }
	HardwareBufferType GetType() override { return kHWBufferIndex; }
	unsigned int GetBufferSize() override { return BufferSize; }

	int GetWidth() override;
	ResourceFormat GetFormat() override { return Format; }
public:
	IDirect3DIndexBuffer9* Buffer;
	unsigned int BufferSize;
	ResourceFormat Format;
};

class VertexBuffer9 : public ImplementResource<IVertexBuffer> 
{
public:
	VertexBuffer9(IDirect3DVertexBuffer9* buffer, unsigned int bufferSize, unsigned int stride, unsigned int offset)
		: Buffer(buffer), BufferSize(bufferSize), Stride(stride), Offset(offset) {}
	VertexBuffer9() :VertexBuffer9(nullptr, 0, 0, 0) {}

	IDirect3DVertexBuffer9*& GetBuffer9() {	return Buffer; }
	HardwareBufferType GetType() override { return kHWBufferVertex; }
	unsigned int GetBufferSize() override {	return BufferSize; }

	unsigned int GetStride() override { return Stride; }
	unsigned int GetOffset() override { return Offset; }
public:
	IDirect3DVertexBuffer9* Buffer;
	unsigned int BufferSize;
	unsigned int Stride, Offset;
};

class ContantBuffer9 : public ImplementResource<IContantBuffer> 
{
public:
	ContantBuffer9(ConstBufferDeclPtr decl);
	ContantBuffer9() :ContantBuffer9(nullptr) {}

	HardwareBufferType GetType() override { return kHWBufferConstant; }
	unsigned int GetBufferSize() override;

	ConstBufferDeclPtr GetDecl() override { return mDecl; }
	char* GetBuffer9();
	void SetBuffer9(char* data, int dataSize);
public:
	ConstBufferDeclPtr mDecl;
	std::vector<char> mBuffer9;
};

/********** Texture **********/
class Texture9 : public ImplementResource<ITexture> 
{
public:
	Texture9(int width, int height, ResourceFormat format, int mipmap);
	Texture9(IDirect3DTexture9* texture);
	//IResourcePtr AsRes() override { return mRes; }

	bool IsCube() const { return mTextureCube != nullptr; }
	bool HasSRV() override { return mTexture != nullptr || mTextureCube != nullptr; }
	void SetSRV9(IDirect3DTexture9* texture);
	IDirect3DTexture9*& GetSRV9() { return mTexture; }
	IDirect3DCubeTexture9*& GetSRVCube9() { return mTextureCube; }

	int GetWidth() override { return mWidth; }
	int GetHeight() override { return mHeight; }
	ResourceFormat GetFormat() override { return mFormat; }
	int GetMipmapCount() override { return mMipCount; }
private:
	D3DSURFACE_DESC GetDesc();
private:
	int mWidth, mHeight, mMipCount;
	ResourceFormat mFormat;
	IDirect3DTexture9 *mTexture;
	IDirect3DCubeTexture9* mTextureCube;
	//IResourcePtr mRes;
};

class RenderTexture9 : public ImplementResource<IRenderTexture>
{
public:
	RenderTexture9();
	RenderTexture9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer);
	ITexturePtr GetColorTexture() override { return mColorTexture; }
	IDirect3DSurface9*& GetColorBuffer9();
	IDirect3DSurface9*& GetDepthStencilBuffer9() { return mDepthStencilBuffer; }
public:
	IDirect3DSurface9* mDepthStencilBuffer;
	IDirect3DSurface9* mColorBuffer;
	Texture9Ptr mColorTexture;
};

class SamplerState9 : public ImplementResource<ISamplerState> 
{
public:
	std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9();
public:
	std::map<D3DSAMPLERSTATETYPE, DWORD> mStates;
};

}