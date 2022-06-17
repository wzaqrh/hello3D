#include "core/rendersys/ogl/framebuffer_ogl.h"
#include "core/rendersys/ogl/ogl_bind.h"
#include "core/base/debug.h"

namespace mir {

void FrameBufferOGL::SetAttachColor(size_t slot, FrameBufferAttachOGLPtr attach) {
	if (attach) {
		if (mAttachColors.size() < slot + 1)
			mAttachColors.resize(slot + 1);
		mAttachColors[slot] = attach;
	}
	else {
		if (slot == mAttachColors.size() - 1) {
			while (!mAttachColors.empty() && mAttachColors.back() == nullptr)
				mAttachColors.pop_back();
		}
	}
}

}