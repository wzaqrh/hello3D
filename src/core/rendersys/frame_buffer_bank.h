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
		Element(const Eigen::Vector3i& fbSize, const std::vector<ResourceFormat>& fmts)
			:mFbSize(fbSize), mFbFormats(fmts)
		{}
		void ReturnAll() {
			mBorrowCount = 0;
		}
		IFrameBufferPtr Borrow(ResourceManager& resMng);
		const std::vector<ResourceFormat>& GetFmts() const { return mFbFormats; }
		const Eigen::Vector3i& GetSize() const { return mFbSize; }
	private:
		Eigen::Vector3i mFbSize;
		std::vector<ResourceFormat> mFbFormats;
		std::vector<IFrameBufferPtr> mFbs;
		size_t mBorrowCount = 0;
	};
public:
	FrameBufferBank(ResourceManager& resMng, const Eigen::Vector3i& fbSize, const std::vector<ResourceFormat>& fmts)
		:mResMng(resMng), mFbSize(fbSize), mFbFormats(fmts) {
		mElements.push_back(CreateInstance<Element>(mFbSize, fmts));
	}
	void ReturnAll() {
		for (auto& iter : mElements)
			iter->ReturnAll();
	}
	IFrameBufferPtr Borrow() { return mElements[0]->Borrow(mResMng); }
	IFrameBufferPtr Borrow(const std::vector<ResourceFormat>& fmts, float size = 1.0f);
private:
	ResourceManager& mResMng;
	Eigen::Vector3i mFbSize;
	std::vector<ResourceFormat> mFbFormats;
	std::vector<std::shared_ptr<Element>> mElements;
};

}