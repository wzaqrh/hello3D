#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/base/math.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/d3d11/predeclare.h"
#include "core/rendersys/d3d11/texture11.h"

namespace mir {

class FrameBuffer11 : public ImplementResource<IFrameBuffer>
{
public:
	FrameBuffer11();
	void Init(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format = kFormatR32G32B32A32Float);
	ITexturePtr GetColorTexture() const override { return mRenderTargetPtr; }

	ID3D11RenderTargetView*& GetColorBuffer11() { return mRenderTargetView; }
	ID3D11DepthStencilView*& GetDepthStencilBuffer11() { return mDepthStencilView; }
private:
	bool InitRenderTexture(ID3D11Device* pDevice);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);

	bool InitDepthStencilTexture(ID3D11Device* pDevice);
	bool InitDepthStencilView(ID3D11Device* pDevice);
private:
	Texture11Ptr mRenderTargetPtr;
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;

	Eigen::Vector2i mSize;
	ResourceFormat mFormat;
};

}