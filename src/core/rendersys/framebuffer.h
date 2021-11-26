#pragma once
#include "core/rendersys/predeclare.h"
#include "core/resource/resource.h"

namespace mir {

interface IFrameBuffer : public IResource
{
	virtual ITexturePtr GetColorTexture() const = 0;
};

}