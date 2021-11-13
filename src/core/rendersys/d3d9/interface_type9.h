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

struct BlobData9 : public IBlobData {
	ID3DXBuffer* mBlob;
public:
	BlobData9(ID3DXBuffer* pBlob);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
};

struct InputLayout9 : public IInputLayout {
public:
	std::vector<D3DVERTEXELEMENT9> mInputDescs;
	IDirect3DVertexDeclaration9* mLayout;
	IResourcePtr mRes;
public:
	InputLayout9();
	IDirect3DVertexDeclaration9*& GetLayout9() { return mLayout; }
	IResourcePtr AsRes() override { return mRes; }
};

struct ConstantTable {
	ID3DXConstantTable* mTable;
	std::vector<D3DXHANDLE> mHandles;
	std::map<std::string, D3DXHANDLE> mHandleByName;
	std::map<std::string, std::shared_ptr<ConstantTable>> mSubByName;
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
};

struct VertexShader9 : public IVertexShader {
	ConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DVertexShader9* mShader;
	IResourcePtr mRes;
public:
	VertexShader9();
	IBlobDataPtr GetBlob() override { return mBlob; }
	IResourcePtr AsRes() override { return mRes; }

	IDirect3DVertexShader9*& GetShader9() { return mShader; }
	void SetConstTable(ID3DXConstantTable* constTable);
};

struct PixelShader9 : public IPixelShader {
	ConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DPixelShader9* mShader;
	IResourcePtr mRes;
public:
	PixelShader9();
	virtual IBlobDataPtr GetBlob() override { return mBlob; }
	IResourcePtr AsRes() override { return mRes; }

	IDirect3DPixelShader9*& GetShader9() { return mShader; }
	void SetConstTable(ID3DXConstantTable* constTable);
};

struct Program9 : public IProgram {
	VertexShader9Ptr mVertex;
	PixelShader9Ptr mPixel;
	IResourcePtr mRes;
public:
	Program9();
	IResourcePtr AsRes() override { return mRes; }
	void SetVertex(VertexShader9Ptr pVertex);
	void SetPixel(PixelShader9Ptr pPixel);
	IVertexShaderPtr GetVertex() override { return mVertex; }
	IPixelShaderPtr GetPixel() override { return mPixel; }
};

/********** HardwareBuffer **********/
struct IndexBuffer9 : public IIndexBuffer {
	IDirect3DIndexBuffer9* Buffer;
	unsigned int BufferSize;
	ResourceFormat Format;
public:
	IndexBuffer9(IDirect3DIndexBuffer9* buffer, unsigned int bufferSize, ResourceFormat format)
		: Buffer(buffer), BufferSize(bufferSize), Format(format) {}
	IndexBuffer9(): Buffer(nullptr), BufferSize(0), Format(kFormatUnknown) {}
public:
	IDirect3DIndexBuffer9*& GetBuffer9() { return Buffer; }
	HardwareBufferType GetType() override { return kHWBufferIndex; }
	unsigned int GetBufferSize() override { return BufferSize; }

	int GetWidth() override;
	ResourceFormat GetFormat() override { return Format; }
};

struct VertexBuffer9 : public IVertexBuffer {
	IDirect3DVertexBuffer9* Buffer;
	unsigned int BufferSize;
	unsigned int Stride, Offset;
public:
	VertexBuffer9(IDirect3DVertexBuffer9* buffer, unsigned int bufferSize, unsigned int stride, unsigned int offset)
		: Buffer(buffer), BufferSize(bufferSize), Stride(stride), Offset(offset) {}
	
	IDirect3DVertexBuffer9*& GetBuffer9() {	return Buffer; }
	HardwareBufferType GetType() override { return kHWBufferVertex; }
	unsigned int GetBufferSize() override {	return BufferSize; }

	unsigned int GetStride() override { return Stride; }
	unsigned int GetOffset() override { return Offset; }
};

struct ContantBuffer9 : public IContantBuffer {
	ConstBufferDeclPtr mDecl;
	std::vector<char> mBuffer9;
public:
	ContantBuffer9(ConstBufferDeclPtr decl);
public:
	HardwareBufferType GetType() override { return kHWBufferConstant; }
	unsigned int GetBufferSize() override;

	ConstBufferDeclPtr GetDecl() override { return mDecl; }
	char* GetBuffer9();
	void SetBuffer9(char* data, int dataSize);
};

/********** Texture **********/
struct Texture9 : public ITexture {
private:
	int mWidth, mHeight, mMipCount;
	ResourceFormat mFormat;
	IDirect3DTexture9 *mTexture;
	IDirect3DCubeTexture9* mTextureCube;
	IResourcePtr mRes;
	std::string mPath;
public:
	Texture9(int width, int height, ResourceFormat format, int mipmap);
	Texture9(IDirect3DTexture9* texture, const std::string& path);
	IResourcePtr AsRes() override { return mRes; }

	bool IsCube() const { return mTextureCube != nullptr; }
	bool HasSRV() override { return mTexture != nullptr || mTextureCube != nullptr; }
	void SetSRV9(IDirect3DTexture9* texture);
	IDirect3DTexture9*& GetSRV9() { return mTexture; }
	IDirect3DCubeTexture9*& GetSRVCube9() { return mTextureCube; }

	const char* GetPath() override { return mPath.c_str(); }
	int GetWidth() override { return mWidth; }
	int GetHeight() override { return mHeight; }
	ResourceFormat GetFormat() override { return mFormat; }
	int GetMipmapCount() override { return mMipCount; }
private:
	D3DSURFACE_DESC GetDesc();
};

struct RenderTexture9 : public IRenderTexture {
	IDirect3DSurface9* mDepthStencilBuffer;
	IDirect3DSurface9* mColorBuffer;
	Texture9Ptr mColorTexture;
public:
	RenderTexture9(Texture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer);
	ITexturePtr GetColorTexture() override { return mColorTexture; }
	IDirect3DSurface9*& GetColorBuffer9();
	IDirect3DSurface9*& GetDepthStencilBuffer9() { return mDepthStencilBuffer; }
};

struct SamplerState9 : public ISamplerState {
	std::map<D3DSAMPLERSTATETYPE, DWORD> mStates;
public:
	std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9();
};

}