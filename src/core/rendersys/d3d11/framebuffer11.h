#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/base/math.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/d3d11/predeclare.h"
#include "core/rendersys/d3d11/texture11.h"

namespace mir {

class FrameBufferAttachColor11 : public IFrameBufferAttachment
{
public:
	FrameBufferAttachColor11();
	void Init(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format);

	ITexturePtr AsTexture() const override { return mRenderTargetPtr; }
	ID3D11RenderTargetView*& AsRTV() { return mRenderTargetView; }
private:
	bool InitRenderTexture(ID3D11Device* pDevice);
	bool InitRenderTextureView(ID3D11Device* pDevice);
	bool InitRenderTargetView(ID3D11Device* pDevice);
private:
	Eigen::Vector2i mSize;
	ResourceFormat mFormat;

	Texture11Ptr mRenderTargetPtr;
	ID3D11Texture2D* mRenderTargetTexture;
	ID3D11ShaderResourceView* mRenderTargetSRV;
	ID3D11RenderTargetView* mRenderTargetView;
};
typedef std::shared_ptr<FrameBufferAttachColor11> FrameBufferAttachColor11Ptr;

class FrameBufferAttachZStencil11 : public IFrameBufferAttachment
{
public:
	FrameBufferAttachZStencil11();
	void Init(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format);

	ITexturePtr AsTexture() const override { return mTexture; }
	ID3D11DepthStencilView*& AsDSV() { return mDepthStencilView; }
private:
	bool InitDepthStencilTexture(ID3D11Device* pDevice);
	bool InitDepthStencilView(ID3D11Device* pDevice);
private:
	Eigen::Vector2i mSize;
	ResourceFormat mFormat;

	ITexturePtr mTexture;
	ID3D11Texture2D* mDepthStencilTexture;
	ID3D11DepthStencilView* mDepthStencilView;
};
typedef std::shared_ptr<FrameBufferAttachZStencil11> FrameBufferAttachZStencil11Ptr;

class FrameBuffer11 : public ImplementResource<IFrameBuffer>
{
public:
	FrameBuffer11();
	void Init(ID3D11Device* pDevice, const Eigen::Vector2i& size, ResourceFormat format);

	size_t GetAttachColorCount() const override { return mAttachColors.size(); }
	IFrameBufferAttachmentPtr GetAttachColor(size_t index) const { return mAttachColors[index]; }
	IFrameBufferAttachmentPtr GetAttachZStencil() const { return mAttachZStencil; }
private:
	std::vector<FrameBufferAttachColor11Ptr> mAttachColors;
	FrameBufferAttachZStencil11Ptr mAttachZStencil;
};

}