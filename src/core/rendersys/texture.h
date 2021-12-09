#pragma once
#include "core/rendersys/predeclare.h"
#include "core/base/base_type.h"
#include "core/resource/resource.h"

namespace mir {

interface ISamplerState : public IResource
{
};

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

}