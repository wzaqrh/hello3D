#pragma once
#include <boost/noncopyable.hpp>
#include "core/predeclare.h"
#include "core/rendersys/hardware_buffer.h"

namespace mir {

class MaterialUniformParameter {
public:

private:
	IContantBufferPtr mBuffer;
};

}