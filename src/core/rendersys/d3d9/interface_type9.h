#pragma once
#include "core/rendersys/interface_type.h"

namespace mir {

typedef std::shared_ptr<struct TBlobDataD3d9> TBlobDataD3d9Ptr;
typedef std::shared_ptr<struct TInputLayout9> TInputLayout9Ptr;
typedef std::shared_ptr<struct TConstantTable> TConstantTablePtr;
typedef std::shared_ptr<struct TVertexShader9> TVertexShader9Ptr;
typedef std::shared_ptr<struct TPixelShader9> TPixelShader9Ptr;
typedef std::shared_ptr<struct TProgram9> TProgram9Ptr;
typedef std::shared_ptr<struct TIndexBuffer9> TIndexBuffer9Ptr;
typedef std::shared_ptr<struct TVertexBuffer9> TVertexBuffer9Ptr;
typedef std::shared_ptr<struct TContantBuffer9> TContantBuffer9Ptr;
typedef std::shared_ptr<struct TTexture9> TTexture9Ptr;
typedef std::shared_ptr<struct TRenderTexture9> TRenderTexture9Ptr;
typedef std::shared_ptr<struct TSamplerState9> TSamplerState9Ptr;

struct TBlobDataD3d9 : public IBlobData {
	ID3DXBuffer* mBlob = nullptr;
public:
	TBlobDataD3d9(ID3DXBuffer* pBlob);
	char* GetBufferPointer() override;
	size_t GetBufferSize() override;
};

struct TInputLayout9 : public IInputLayout {
public:
	std::vector<D3DVERTEXELEMENT9> mInputDescs;
	IDirect3DVertexDeclaration9* mLayout = nullptr;
	IResourcePtr mRes;
public:
	TInputLayout9();
	IDirect3DVertexDeclaration9*& GetLayout9();
	IResourcePtr AsRes() override {
		return mRes;
	}
};

struct TConstantTable {
	ID3DXConstantTable* mTable = nullptr;
	std::vector<D3DXHANDLE> mHandles;
	std::map<std::string, D3DXHANDLE> mHandleByName;
	std::map<std::string, std::shared_ptr<TConstantTable>> mSubByName;
public:
	void ReInit();
	void Init(ID3DXConstantTable* constTable, D3DXHANDLE handle = NULL, int count = 0);
	ID3DXConstantTable* get();
	size_t size() const;
	D3DXHANDLE GetHandle(size_t pos) const;
	D3DXHANDLE GetHandle(const std::string& name) const;
	std::shared_ptr<TConstantTable> At(const std::string& name);
public:
	void SetValue(IDirect3DDevice9* device, char* buffer9, TConstBufferDeclElement& elem);
	void SetValue(IDirect3DDevice9* device, char* buffer9, TConstBufferDecl& decl);
};

struct TVertexShader9 : public IVertexShader {
	TConstantTable mConstTable;
	IBlobDataPtr mBlob,mErrBlob;
	IDirect3DVertexShader9* mShader = nullptr;
	IResourcePtr mRes;
public:
	TVertexShader9();
	IBlobDataPtr GetBlob() override;
	IResourcePtr AsRes() override {
		return mRes;
	}

	IDirect3DVertexShader9*& GetShader9();
	void SetConstTable(ID3DXConstantTable* constTable);
};

struct TPixelShader9 : public IPixelShader {
	TConstantTable mConstTable;
	IBlobDataPtr mBlob, mErrBlob;
	IDirect3DPixelShader9* mShader = nullptr;
	IResourcePtr mRes;
public:
	TPixelShader9();
	virtual IBlobDataPtr GetBlob() override;
	IResourcePtr AsRes() override {
		return mRes;
	}

	IDirect3DPixelShader9*& GetShader9();
	void SetConstTable(ID3DXConstantTable* constTable);
};

struct TProgram9 : public IProgram {
	TVertexShader9Ptr mVertex;
	TPixelShader9Ptr mPixel;
	IResourcePtr mRes;
public:
	TProgram9();
	IResourcePtr AsRes() override {
		return mRes;
	}
	void SetVertex(TVertexShader9Ptr pVertex);
	void SetPixel(TPixelShader9Ptr pPixel);
	IVertexShaderPtr GetVertex() override {
		return mVertex;
	}
	IPixelShaderPtr GetPixel() override {
		return mPixel;
	}
};

/********** HardwareBuffer **********/
struct TIndexBuffer9 : public IIndexBuffer {
	IDirect3DIndexBuffer9* buffer;
	unsigned int bufferSize;
	DXGI_FORMAT format;
public:
	TIndexBuffer9(IDirect3DIndexBuffer9* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:buffer(__buffer), bufferSize(__bufferSize), format(__format) {};
	TIndexBuffer9() :buffer(nullptr), bufferSize(0), format(DXGI_FORMAT_UNKNOWN) {};
public:
	IDirect3DIndexBuffer9*& GetBuffer9();
	enHardwareBufferType GetType() override {
		return E_HWBUFFER_INDEX;
	}
	unsigned int GetBufferSize() override;

	int GetWidth() override;
	DXGI_FORMAT GetFormat() override;
};

struct TVertexBuffer9 : public IVertexBuffer {
	IDirect3DVertexBuffer9* buffer;
	unsigned int bufferSize;
	unsigned int stride, offset;
public:
	TVertexBuffer9(IDirect3DVertexBuffer9* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:buffer(__buffer), bufferSize(__bufferSize), stride(__stride), offset(__offset) {};
	
	IDirect3DVertexBuffer9*& GetBuffer9();
	enHardwareBufferType GetType() override {
		return E_HWBUFFER_VERTEX;
	}
	unsigned int GetBufferSize() override;

	unsigned int GetStride() override;
	unsigned int GetOffset() override;
};

struct TContantBuffer9 : public IContantBuffer {
	TConstBufferDeclPtr mDecl;
	std::vector<char> mBuffer9;
public:
	TContantBuffer9(TConstBufferDeclPtr decl);
public:
	enHardwareBufferType GetType() override {
		return E_HWBUFFER_CONSTANT;
	}
	unsigned int GetBufferSize() override;

	TConstBufferDeclPtr GetDecl() override;
	char* GetBuffer9();
	void SetBuffer9(char* data, int dataSize);
};

/********** Texture **********/
struct TTexture9 : public ITexture {
private:
	int mWidth = 0, mHeight = 0, mMipCount = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_UNKNOWN;
	IDirect3DTexture9 *mTexture;
	IDirect3DCubeTexture9* mTextureCube;
	IResourcePtr mRes;
	std::string mPath;
public:
	TTexture9(int width, int height, DXGI_FORMAT format, int mipmap);
	TTexture9(IDirect3DTexture9 *__texture, const std::string& __path);
	IResourcePtr AsRes() override {
		return mRes;
	}

	bool HasSRV() override {
		return mTexture != nullptr || mTextureCube != nullptr;
	}
	void SetSRV9(IDirect3DTexture9* __texture);
	IDirect3DTexture9*& GetSRV9() {
		return mTexture;
	}
	IDirect3DCubeTexture9*& GetSRVCube9() {
		return mTextureCube;
	}

	const char* GetPath() override;
	int GetWidth() override;
	int GetHeight() override;
	DXGI_FORMAT GetFormat() override;
	int GetMipmapCount() override;
private:
	D3DSURFACE_DESC GetDesc();
};

struct TRenderTexture9 : public IRenderTexture {
	IDirect3DSurface9* mDepthStencilBuffer;
	IDirect3DSurface9* mColorBuffer;
	TTexture9Ptr mColorTexture;
public:
	TRenderTexture9(TTexture9Ptr colorTexture, IDirect3DSurface9* depthStencilBuffer);
	ITexturePtr GetColorTexture() override;
	IDirect3DSurface9*& GetColorBuffer9();
	IDirect3DSurface9*& GetDepthStencilBuffer9();
};

struct TSamplerState9 : public ISamplerState {
	std::map<D3DSAMPLERSTATETYPE, DWORD> mStates;
public:
	std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9();
};

}