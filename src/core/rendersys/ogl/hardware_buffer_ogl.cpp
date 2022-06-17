#include "core/rendersys/ogl/hardware_buffer_ogl.h"

namespace mir {

int IndexBufferOGL::GetWidth() const
{
	return BytePerPixel(mFormat);
}

}