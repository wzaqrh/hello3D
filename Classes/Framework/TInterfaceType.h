#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <dinput.h>
#include <dxerr.h>
#include "std.h"

struct TProgram {
	ID3D11VertexShader* mVertexShader = nullptr;
	ID3D11PixelShader* mPixelShader = nullptr;
	ID3DBlob* mVSBlob = nullptr;
};
typedef std::shared_ptr<TProgram> TProgramPtr;

struct THardwareBuffer {
	ID3D11Buffer* buffer;
	unsigned int bufferSize;
public:
	THardwareBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :buffer(__buffer), bufferSize(__bufferSize) {};
	THardwareBuffer() :buffer(nullptr), bufferSize(0) {};
};

struct TVertexBuffer : public THardwareBuffer {
	unsigned int stride, offset;
public:
	TVertexBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize, unsigned int __stride, unsigned int __offset)
		:THardwareBuffer(__buffer, __bufferSize), stride(__stride), offset(__offset) {};
	TVertexBuffer() :stride(0), offset(0) {};
};
typedef std::shared_ptr<TVertexBuffer> TVertexBufferPtr;

struct TIndexBuffer : public THardwareBuffer {
	DXGI_FORMAT format;
public:
	TIndexBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize, DXGI_FORMAT __format)
		:THardwareBuffer(__buffer, __bufferSize), format(__format) {};
	TIndexBuffer() :format(DXGI_FORMAT_UNKNOWN) {};
	int GetWidth();
};
typedef std::shared_ptr<TIndexBuffer> TIndexBufferPtr;

struct TContantBuffer : public THardwareBuffer {
	TContantBuffer() {}
	TContantBuffer(ID3D11Buffer* __buffer, unsigned int __bufferSize) :THardwareBuffer(__buffer, __bufferSize) {}
};
typedef std::shared_ptr<TContantBuffer> TContantBufferPtr;


enum enTextureType {
	E_TEXTURE_DIFFUSE,
	E_TEXTURE_SPECULAR,
	E_TEXTURE_NORMAL
};
enum enTexturePbrType {
	E_TEXTURE_PBR_ALBEDO,
	E_TEXTURE_PBR_NORMAL,
	E_TEXTURE_PBR_METALNESS,
	E_TEXTURE_PBR_ROUGHNESS,
	E_TEXTURE_PBR_AO
};
#define E_TEXTURE_DEPTH_MAP 8
struct TTexture {
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	std::string path;
	ID3D11ShaderResourceView *texture;
public:
	TTexture();
	TTexture(std::string __path, ID3D11ShaderResourceView* __texture);
};


struct TTextureBySlot {
	std::vector<TTexture> textures;
public:
	void clear();
	void push_back(const TTexture& texture);
	bool empty() const;
	size_t size() const;
	void swap(TTextureBySlot& other);
	void resize(size_t size);
	const TTexture& At(size_t pos) const;
	TTexture& At(size_t pos);
	const TTexture& operator[](size_t pos) const;
	TTexture& operator[](size_t pos);
	std::vector<ID3D11ShaderResourceView*> GetTextureViews() const;
};


class TRenderTexture {
public:
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	DXGI_FORMAT mFormat;
public:
	TRenderTexture(ID3D11Device* pDevice, int width, int height, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT);
private:
	bool InitRenderTexture(ID3D11Device* pDevice, int width, int height);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice, int width, int height);
	bool InitDepthStencilView(ID3D11Device* pDevice);
};
typedef std::shared_ptr<TRenderTexture> TRenderTexturePtr;


struct TMaterial;
typedef std::shared_ptr<TMaterial> TMaterialPtr;

struct TRenderOperation {
	TMaterialPtr mMaterial;
	TVertexBufferPtr mVertexBuffer;
	TIndexBufferPtr mIndexBuffer;
	TTextureBySlot mTextures;
	XMMATRIX mWorldTransform;
public:
	TRenderOperation();
};

struct TRenderOperationQueue {
	std::vector<TRenderOperation> mOps;
public:
	void clear();
	void push_back(const TRenderOperation& op);
	size_t size() const;
	TRenderOperation& At(size_t pos);
	const TRenderOperation& At(size_t pos) const;
	TRenderOperation& operator[](size_t pos);
	const TRenderOperation& operator[](size_t pos) const;
};

struct IRenderable {
	virtual int GenRenderOperation(TRenderOperationQueue& opList) = 0;
};