#pragma once
#include "core/rendersys/predeclare.h"
#include "core/rendersys/base_type.h"
#include "core/resource/resource.h"

namespace mir {

interface ISamplerState : public IResource
{
};

interface ITexture : public IResource
{
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual ResourceFormat GetFormat() const = 0;
	virtual HWMemoryUsage GetUsage() const = 0;
	virtual int GetFaceCount() const = 0;
	virtual int GetMipmapCount() const = 0;
	virtual bool IsAutoGenMipmap() const = 0;
};

}