#pragma once
#include <windows.h>
#include <d3d11.h>
#include "core/base/math.h"
#include "core/rendersys/base_type.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/d3d11/predeclare.h"
#include "core/rendersys/d3d11/texture11.h"

namespace mir {

interface FrameBufferAttach11: public IFrameBufferAttachment
{
	virtual ID3D11RenderTargetView* AsRTV() = 0;
	virtual ID3D11DepthStencilView* AsDSV() = 0;
	virtual ID3D11ShaderResourceView* AsSRV() = 0;
}; 
typedef std::shared_ptr<FrameBufferAttach11> FrameBufferAttach11Ptr;

class FrameBufferAttachByTexture11 : public FrameBufferAttach11
{
public:
	FrameBufferAttachByTexture11(Texture11Ptr texture) :mTexture(texture) {}
	ITexturePtr AsTexture() const override { return mTexture; }
	ID3D11RenderTargetView* AsRTV() override { return mTexture->AsRTV(); }
	ID3D11DepthStencilView* AsDSV() override { return mTexture->AsDSV(); }
	ID3D11ShaderResourceView* AsSRV() override { return mTexture->AsSRV(); }
private:
	Texture11Ptr mTexture;
};
typedef std::shared_ptr<FrameBufferAttachByTexture11> FrameBufferAttachByTexture11Ptr;

class FrameBufferAttachByView : public FrameBufferAttach11
{
public:
	FrameBufferAttachByView(ID3D11RenderTargetView* pRTV) :mRTV(pRTV) {}
	FrameBufferAttachByView(ID3D11DepthStencilView* pDSV) :mDSV(pDSV) {}
	ITexturePtr AsTexture() const override { return nullptr; }
	ID3D11RenderTargetView* AsRTV() override { return mRTV; }
	ID3D11DepthStencilView* AsDSV() override { return mDSV; }
	ID3D11ShaderResourceView* AsSRV() override { return nullptr; }
private:
	ID3D11RenderTargetView* mRTV = nullptr;
	ID3D11DepthStencilView* mDSV = nullptr;
};
typedef std::shared_ptr<FrameBufferAttachByView> FrameBufferAttachByViewPtr;

class FrameBuffer11 : public ImplementResource<IFrameBuffer>
{
public:
	void SetAttachColor(size_t slot, FrameBufferAttach11Ptr attach);
	void SetAttachZStencil(FrameBufferAttach11Ptr attach) {
		mAttachZStencil = attach;
	}

	void SetSize(Eigen::Vector2i size) { 
		mSize = size;
	}
	Eigen::Vector2i GetSize() const override { return mSize; }
	size_t GetAttachColorCount() const override { return mAttachColors.size(); }
	IFrameBufferAttachmentPtr GetAttachColor(size_t index) const override { 
		return !mAttachColors.empty() ? mAttachColors[index] : nullptr; 
	}
	IFrameBufferAttachmentPtr GetAttachZStencil() const override { return mAttachZStencil; }

	std::vector<ID3D11ShaderResourceView*> AsSRVs() const;
	std::vector<ID3D11RenderTargetView*> AsRTVs() const;
	ID3D11DepthStencilView* AsDSV() const { return mAttachZStencil ? mAttachZStencil->AsDSV() : nullptr; }
private:
	std::vector<FrameBufferAttach11Ptr> mAttachColors;
	FrameBufferAttach11Ptr mAttachZStencil;
	Eigen::Vector2i mSize;
};

}