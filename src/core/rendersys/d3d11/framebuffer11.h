#pragma once
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#include "core/base/math.h"
#include "core/rendersys/framebuffer.h"
#include "core/rendersys/d3d11/predeclare.h"
#include "core/rendersys/d3d11/texture11.h"

namespace mir {

class FrameBuffer11Attach : public IFrameBufferAttachment
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	FrameBuffer11Attach(Texture11Ptr texture) :mTexture(texture) {}
	ITexturePtr AsTexture() const override { return mTexture; }
	const ComPtr<ID3D11RenderTargetView>& AsRTV() { return mTexture->AsRTV(); }
	const ComPtr<ID3D11DepthStencilView>& AsDSV() { return mTexture->AsDSV(); }
	const ComPtr<ID3D11ShaderResourceView>& AsSRV() { return mTexture->AsSRV(); }
private:
	Texture11Ptr mTexture;
};
typedef std::shared_ptr<FrameBuffer11Attach> FrameBuffer11AttachPtr;

class FrameBuffer11AttachFactory 
{
public:
	static FrameBuffer11AttachPtr CreateColorAttachment(const ComPtr<ID3D11Device>& pDevice, const Eigen::Vector3i& size, ResourceFormat format);
	static FrameBuffer11AttachPtr CreateZStencilAttachment(const ComPtr<ID3D11Device>& pDevice, const Eigen::Vector2i& size, ResourceFormat format);
};

class FrameBuffer11 : public ImplementResource<IFrameBuffer>
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void SetAttachColor(size_t slot, IFrameBufferAttachmentPtr attach) override;
	void SetAttachZStencil(IFrameBufferAttachmentPtr attach) override;

	void SetSize(Eigen::Vector2i size) { mSize = size; }
	Eigen::Vector2i GetSize() const override { return mSize; }
	size_t GetAttachColorCount() const override { return mAttachColors.size(); }
	IFrameBufferAttachmentPtr GetAttachColor(size_t index) const override { return !mAttachColors.empty() ? mAttachColors[index] : nullptr; }
	IFrameBufferAttachmentPtr GetAttachZStencil() const override { return mAttachZStencil; }

	std::vector<ID3D11ShaderResourceView*> AsSRVs() const;
	std::vector<ID3D11RenderTargetView*> AsRTVs() const;
	ID3D11DepthStencilView* AsDSV() const { return mAttachZStencil ? mAttachZStencil->AsDSV().Get() : nullptr; }
private:
	std::vector<FrameBuffer11AttachPtr> mAttachColors;
	FrameBuffer11AttachPtr mAttachZStencil;
	Eigen::Vector2i mSize;
};

}