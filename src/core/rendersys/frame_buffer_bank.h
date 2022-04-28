#pragma once
#include "core/rendersys/predeclare.h"
#include "core/resource/predeclare.h"
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/base/base_type.h"

namespace mir {

class FrameBufferBank
{
	struct Element {
		Element(const Eigen::Vector2i& fbSize, const std::vector<ResourceFormat>& fmts)
			:mFbSize(fbSize), mFbFormats(fmts)
		{}
		void ReturnAll() {
			mBorrowCount = 0;
		}
		IFrameBufferPtr Borrow(ResourceManager& resMng);
		const std::vector<ResourceFormat>& GetFmts() const { return mFbFormats; }
	private:
		Eigen::Vector2i mFbSize;
		std::vector<ResourceFormat> mFbFormats;
		std::vector<IFrameBufferPtr> mFbs;
		size_t mBorrowCount = 0;
	};
public:
	FrameBufferBank(ResourceManager& resMng, const Eigen::Vector2i& fbSize, const std::vector<ResourceFormat>& fmts)
		:mResMng(resMng), mFbSize(fbSize) {
		mElements.push_back(CreateInstance<Element>(mFbSize, fmts));
	}
	void ReturnAll() {
		for (auto& iter : mElements)
			iter->ReturnAll();
	}
	IFrameBufferPtr Borrow() { return mElements[0]->Borrow(mResMng); }
	IFrameBufferPtr Borrow(const std::vector<ResourceFormat>& fmts);
private:
	ResourceManager& mResMng;
	Eigen::Vector2i mFbSize;
	std::vector<std::shared_ptr<Element>> mElements;
};

}