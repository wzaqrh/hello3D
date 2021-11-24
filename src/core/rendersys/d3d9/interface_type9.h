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
	const char* GetBufferPointer() const override;
	size_t GetBufferSize() const override;
public:
	ID3DXBuffer* mBlob;
};

class InputLayout9 : public ImplementResource<IInputLayout> 
{
public:
	InputLayout9();
	IDirect3DVertexDeclaration9*& GetLayout9() { return mLayout; }
public:
	std::vector<D3DVERTEXELEMENT9> mInputDescs;
	IDirect3DVertexDeclaration9* mLayout;
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
	HardwareBufferType GetType() const override { return kHWBufferIndex; }
	unsigned int GetBufferSize() const override { return BufferSize; }

	int GetWidth() const override;
	ResourceFormat GetFormat() const override { return Format; }
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
	HardwareBufferType GetType() const override { return kHWBufferVertex; }
	unsigned int GetBufferSize() const override {	return BufferSize; }

	unsigned int GetStride() const override { return Stride; }
	unsigned int GetOffset() const override { return Offset; }
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

	HardwareBufferType GetType() const override { return kHWBufferConstant; }
	unsigned int GetBufferSize() const override;

	ConstBufferDeclPtr GetDecl() const override { return mDecl; }
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

	bool IsCube() const { return mTextureCube != nullptr; }
	bool HasSRV() const override { return mTexture != nullptr || mTextureCube != nullptr; }
	void SetSRV9(IDirect3DTexture9* texture);
	IDirect3DTexture9*& GetSRV9() { return mTexture; }
	IDirect3DCubeTexture9*& GetSRVCube9() { return mTextureCube; }

	int GetWidth() const override { return mWidth; }
	int GetHeight() const override { return mHeight; }
	ResourceFormat GetFormat() const override { return mFormat; }
	int GetMipmapCount() const override { return mMipCount; }
	int GetFaceCount() const override { return 1; }
	bool IsAutoGenMipmap() const override { return false; }
private:
	D3DSURFACE_DESC GetDesc();
private:
	int mWidth, mHeight, mMipCount;
	ResourceFormat mFormat;
	IDirect3DTexture9 *mTexture;
	IDirect3DCubeTexture9* mTextureCube;
};

class RenderTexture9 : public ImplementResource<IRenderTexture>
{
public:
	RenderTexture9();
	RenderTexture9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer);
	ITexturePtr GetColorTexture() const override { return mColorTexture; }
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