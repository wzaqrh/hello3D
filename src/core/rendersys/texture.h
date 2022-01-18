#pragma once
#include "core/rendersys/predeclare.h"
#include "core/base/base_type.h"
#include "core/base/tpl/vector.h"
#include "core/resource/resource.h"

namespace mir {

interface ITexture : public IResource
{
	virtual Eigen::Vector2i GetSize() const = 0;
	virtual Eigen::Vector2i GetRealSize() const = 0;
	virtual ResourceFormat GetFormat() const = 0;
	virtual HWMemoryUsage GetUsage() const = 0;
	virtual int GetFaceCount() const = 0;
	virtual int GetMipmapCount() const = 0;
	virtual bool IsAutoGenMipmap() const = 0;

	int GetWidth() const { return GetSize().x(); }
	int GetHeight() const { return GetSize().y(); }
};

struct TextureVector : public tpl::Vector<std::shared_ptr<ITexture>>
{
	void Merge(const TextureVector& other) {
		if (mElements.size() < other.mElements.size())
			mElements.resize(other.mElements.size());

		for (size_t i = 0; i < other.mElements.size(); ++i) {
			if (other.mElements[i] && other.mElements[i]->IsLoaded()) {
				mElements[i] = other.mElements[i];
			}
		}
	}
	bool IsLoaded() const {
		for (size_t i = 0; i < mElements.size(); ++i) {
			if (mElements[i] && !mElements[i]->IsLoaded())
				return false;
		}
		return true;
	}
};

}