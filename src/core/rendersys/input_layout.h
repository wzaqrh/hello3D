#pragma once
#include "core/base/base_type.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/resource.h"

namespace mir {

enum LayoutInputClass {
	kLayoutInputPerVertexData = 0,
	kLayoutInputPerInstanceData = 1
};
struct LayoutInputElement {
	std::string SemanticName;
	unsigned SemanticIndex;
	ResourceFormat Format;
	unsigned InputSlot;
	unsigned AlignedByteOffset;
	LayoutInputClass InputSlotClass;
	unsigned InstanceDataStepRate;
};

interface IInputLayout : public IResource
{
};

}