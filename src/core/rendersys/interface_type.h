#pragma once
#include "core/rendersys/interface_type_pred.h"

/********** Program **********/
MIDL_INTERFACE("2695D081-9638-4E9F-8457-0DEE61E64CC2")
IBlobData : public IUnknown {
	virtual STDMETHODIMP_(char*) GetBufferPointer() = 0;
	virtual STDMETHODIMP_(size_t) GetBufferSize() = 0;
};

struct INHERIT_COM("96C6C5C1-66E3-41A5-B1C5-352F744A643D")
TBlobDataStd : public ComBase<IBlobData> {
	std::vector<char> mBuffer;
public:
	TBlobDataStd(const std::vector<char>& buffer);
	STDMETHODIMP_(char*) GetBufferPointer() override;
	STDMETHODIMP_(size_t) GetBufferSize() override;
};

MIDL_INTERFACE("5A454316-2990-43F4-80BF-FBEE7CF6CA64")
IInputLayout : public IUnknown {
	//virtual ID3D11InputLayout*& GetLayout11();
	//virtual IDirect3DVertexDeclaration9*& GetLayout9();
	virtual STDMETHODIMP_(IResourcePtr) AsRes() = 0;
};

MIDL_INTERFACE("777F6B5E-8D44-4B8C-97FA-650E8941F5DD") 
IVertexShader : public IUnknown{
	virtual STDMETHODIMP_(IBlobDataPtr) GetBlob() = 0;
	//virtual ID3D11VertexShader*& GetShader11();
	//virtual IDirect3DVertexShader9*& GetShader9();
	virtual STDMETHODIMP_(IResourcePtr) AsRes() = 0;
};

MIDL_INTERFACE("71327DD1-B0CF-4D2A-BF7F-E2BFE7BFCF6C") 
IPixelShader : public IUnknown{
public:
	virtual STDMETHODIMP_(IBlobDataPtr) GetBlob() = 0;
	//virtual ID3D11PixelShader*& GetShader11();
	//virtual IDirect3DPixelShader9*& GetShader9();
	virtual STDMETHODIMP_(IResourcePtr) AsRes() = 0;
};

MIDL_INTERFACE("045503F0-A83C-4C83-98B4-C8E0ADE5B501")
IProgram : public IUnknown{
	virtual STDMETHODIMP_(IVertexShaderPtr) GetVertex() = 0;
	virtual STDMETHODIMP_(IPixelShaderPtr) GetPixel() = 0;
	virtual STDMETHODIMP_(IResourcePtr) AsRes() = 0;
};

/********** HardwareBuffer **********/
enum enHardwareBufferType {
	E_HWBUFFER_CONSTANT,
	E_HWBUFFER_VERTEX,
	E_HWBUFFER_INDEX
};
MIDL_INTERFACE("E1E1C66A-3EBE-4CCA-9D31-1B8F659FB1F4")
IHardwareBuffer : public IUnknown {
	virtual STDMETHODIMP_(enHardwareBufferType) GetType() = 0;
	//virtual ID3D11Buffer*& GetBuffer11();
	virtual STDMETHODIMP_(unsigned int) GetBufferSize() = 0;
};

MIDL_INTERFACE("D7760DD8-E04F-4250-9D27-3C7621176481") 
IVertexBuffer : public IHardwareBuffer {
	//virtual enHardwareBufferType GetType() override final;

	//virtual IDirect3DVertexBuffer9*& GetBuffer9();
	virtual STDMETHODIMP_(unsigned int) GetStride() = 0;
	virtual STDMETHODIMP_(unsigned int) GetOffset() = 0;
};

MIDL_INTERFACE("2FE3BA8E-76E7-4C62-859D-6498B33F2208") 
IIndexBuffer : public IHardwareBuffer {
	//virtual enHardwareBufferType GetType() override final;

	//virtual IDirect3DIndexBuffer9*& GetBuffer9();
	virtual STDMETHODIMP_(int) GetWidth() = 0;
	virtual STDMETHODIMP_(DXGI_FORMAT) GetFormat() = 0;
	int GetCount();
};

MIDL_INTERFACE("269D01F6-D1C2-44EF-9FF1-5A272361915E") 
IContantBuffer : public IHardwareBuffer {
	//virtual enHardwareBufferType GetType() override final;

	//virtual void* GetBuffer9();
	virtual STDMETHODIMP_(TConstBufferDeclPtr) GetDecl() = 0;
};

/********** Texture **********/
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
#define E_TEXTURE_ENV 9

MIDL_INTERFACE("9D909FBA-F07A-47C6-A26C-579F008785AE") 
ITexture : public IUnknown{
	//virtual void SetSRV11(ID3D11ShaderResourceView* __texture) {};
	//virtual ID3D11ShaderResourceView*& GetSRV11();
	
	//virtual void SetSRV9(IDirect3DTexture9* __texture) {};
	//virtual IDirect3DTexture9*& GetSRV9();
	//virtual IDirect3DCubeTexture9*& GetSRVCube9();
	virtual STDMETHODIMP_(IResourcePtr) AsRes() = 0;
	virtual STDMETHODIMP_(bool) HasSRV() = 0;

	virtual STDMETHODIMP_(const char*) GetPath() = 0;
	virtual STDMETHODIMP_(int) GetWidth() = 0;
	virtual STDMETHODIMP_(int) GetHeight() = 0;
	virtual STDMETHODIMP_(DXGI_FORMAT) GetFormat() = 0;
	virtual STDMETHODIMP_(int) GetMipmapCount() = 0;
};

MIDL_INTERFACE("981ACD59-B868-43E5-A179-E20FB65EF83B") 
IRenderTexture : public IUnknown {
	virtual STDMETHODIMP_(ITexturePtr) GetColorTexture() = 0;

	//virtual ID3D11RenderTargetView*& GetColorBuffer11();
	//virtual ID3D11DepthStencilView*& GetDepthStencilBuffer11();

	//virtual IDirect3DSurface9*& GetColorBuffer9();
	//virtual IDirect3DSurface9*& GetDepthStencilBuffer9();
};

MIDL_INTERFACE("AAA0B36E-55F2-48EC-8BF9-F2595EF275BF") 
ISamplerState : public IUnknown{
	//virtual ID3D11SamplerState*& GetSampler11();
	//virtual std::map<D3DSAMPLERSTATETYPE, DWORD>& GetSampler9();
};