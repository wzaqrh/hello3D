#pragma once
#include "TInterfaceType.h"

struct TTexture9 : public ITexture {
private:
	std::string path;
	IDirect3DTexture9 *texture;
public:
	TTexture9(IDirect3DTexture9* __texture, std::string __path);
	virtual IUnknown*& GetDeviceObject() override;

	virtual void SetSRV9(IDirect3DTexture9* __texture);
	virtual IDirect3DTexture9*& GetSRV9();

	const std::string& GetPath() const;
	int GetWidth();
	int GetHeight();
	DXGI_FORMAT GetFormat();
private:
	D3DSURFACE_DESC GetDesc();
};
typedef std::shared_ptr<TTexture9> TTexture9Ptr;

