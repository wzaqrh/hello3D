#include <boost/format.hpp>
#include "core/rendersys/frame_buffer_bank.h"
#include "core/resource/resource_manager.h"
#include "core/base/debug.h"
#include "core/base/macros.h"

namespace mir {

IFrameBufferPtr FrameBufferBank::Element::Borrow(ResourceManager& resMng) {
	IFrameBufferPtr res = nullptr;
	if (mBorrowCount >= mFbs.size()) {
		for (size_t i = 0; i < 4; ++i) {
			auto tmpfb = resMng.CreateFrameBuffer(__LaunchSync__, mFbSize, mFbFormats);
			DEBUG_SET_PRIV_DATA(tmpfb, (boost::format("TempFrameBufferManager.temp%d") % mFbs.size()).str().c_str());
			mFbs.push_back(tmpfb);
		}
	}
	res = mFbs[mBorrowCount];
	mBorrowCount++;
	return res;
}

IFrameBufferPtr FrameBufferBank::Borrow(const std::vector<ResourceFormat>& fmts, float size) {
	if (fmts.empty() && size == 1.0f) return mElements[0]->Borrow(mResMng);

	Eigen::Vector3i fbSize(mFbSize.x() * size, mFbSize.y() * size, mFbSize.z());
	const std::vector<ResourceFormat>* fbFmts = IF_AND_OR(!fmts.empty(), &fmts, &mFbFormats);

	for (auto& iter : mElements)
		if (iter->GetFmts() == *fbFmts && iter->GetSize() == fbSize)
			return iter->Borrow(mResMng);

	mElements.push_back(CreateInstance<Element>(fbSize, *fbFmts));
	return mElements.back()->Borrow(mResMng);
}

}