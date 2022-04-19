#include "core/rendersys/d3d11/framebuffer11.h"
#include "core/base/debug.h"

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

std::vector<ID3D11RenderTargetView*> FrameBuffer11::AsRTVs() const {
	std::vector<ID3D11RenderTargetView*> vec;
	for (size_t i = 0; i < mAttachColors.size(); ++i)
		vec.push_back(mAttachColors[i]->AsRTV());
	return vec;
}

std::vector<ID3D11ShaderResourceView*> FrameBuffer11::AsSRVs() const {
	std::vector<ID3D11ShaderResourceView*> vec;
	for (size_t i = 0; i < mAttachColors.size(); ++i)
		vec.push_back(mAttachColors[i]->AsSRV());
	if (mAttachZStencil) 
		vec.push_back(mAttachZStencil->AsSRV());
	return vec;
}

}