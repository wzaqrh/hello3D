#include "core/rendersys/d3d11/framebuffer11.h"
#include "core/rendersys/d3d11/d3d_utils.h"
#include "core/base/debug.h"
#include "core/base/macros.h"

namespace mir {

void FrameBuffer11::SetAttachColor(size_t slot, IFrameBufferAttachmentPtr attach) {
	if (attach) {
		if (mAttachColors.size() < slot + 1)
			mAttachColors.resize(slot + 1);
		mAttachColors[slot] = std::static_pointer_cast<FrameBufferAttach11>(attach);
	}
	else {
		if (slot == mAttachColors.size() - 1) {
			while (!mAttachColors.empty() && mAttachColors.back() == nullptr)
				mAttachColors.pop_back();
		}
	}
}

void FrameBuffer11::SetAttachZStencil(IFrameBufferAttachmentPtr attach) { 
	mAttachZStencil = std::static_pointer_cast<FrameBufferAttach11>(attach); 
}

std::vector<ID3D11RenderTargetView*> FrameBuffer11::AsRTVs() const {
	std::vector<ID3D11RenderTargetView*> vec;
	for (size_t i = 0; i < mAttachColors.size(); ++i)
		vec.push_back(mAttachColors[i]->AsRTV().Get());
	return vec;
}

std::vector<ID3D11ShaderResourceView*> FrameBuffer11::AsSRVs() const {
	std::vector<ID3D11ShaderResourceView*> vec;
	for (size_t i = 0; i < mAttachColors.size(); ++i)
		vec.push_back(mAttachColors[i]->AsSRV().Get());
	if (mAttachZStencil) 
		vec.push_back(mAttachZStencil->AsSRV().Get());
	return vec;
}

static Texture11Ptr _CreateColorAttachTexture(const ComPtr<ID3D11Device>& pDevice, const Eigen::Vector3i& size, ResourceFormat format)
{
	constexpr size_t faceCount = 1;

	Texture11Ptr texture = CreateInstance<Texture11>();
	texture->Init(format, kHWUsageDefault, size.x(), size.y(), faceCount, size.z(), true);
	if (!texture->InitTex(pDevice)) return nullptr;
	if (!texture->InitSRV(pDevice)) return nullptr;
	if (!texture->InitRTV(pDevice)) return nullptr;

	texture->SetLoaded();
	return texture;
}
FrameBufferAttachByTexture11Ptr FrameBufferAttachFactory::CreateColorAttachment(const ComPtr<ID3D11Device>& pDevice, const Eigen::Vector3i& size, ResourceFormat format)
{
	return (format != kFormatUnknown) ?  CreateInstance<FrameBufferAttachByTexture11>(_CreateColorAttachTexture(pDevice, size, format)) : nullptr;
}

static Texture11Ptr _CreateZStencilAttachTexture(const ComPtr<ID3D11Device>& pDevice, const Eigen::Vector2i& size, ResourceFormat format)
{
	BOOST_ASSERT(IsDepthStencil(format));
	BOOST_ASSERT(format == kFormatD24UNormS8UInt
		|| format == kFormatD32Float
		|| format == kFormatD16UNorm);

	constexpr size_t faceCount = 1;
	constexpr size_t mipCount = 1;

	Texture11Ptr texture = CreateInstance<Texture11>();
	texture->Init(kFormatD24UNormS8UInt, kHWUsageDefault, size.x(), size.y(), faceCount, mipCount, true);
	if (!texture->InitTex(pDevice)) return nullptr;
	if (!texture->InitDSV(pDevice)) return nullptr;
	if (!texture->InitSRV(pDevice)) return nullptr;

	texture->SetLoaded();
	return texture;
}
FrameBufferAttachByTexture11Ptr FrameBufferAttachFactory::CreateZStencilAttachment(const ComPtr<ID3D11Device>& pDevice, const Eigen::Vector2i& size, ResourceFormat format)
{
	return (format != kFormatUnknown) ? CreateInstance<FrameBufferAttachByTexture11>(_CreateZStencilAttachTexture(pDevice, size, format)) : nullptr;
}

}