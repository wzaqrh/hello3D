#include "core/rendersys/hardware_buffer.h"

namespace mir {

size_t GetCbElementTypeByteWidth(CbElementType type) {
	switch (type)
	{
	case kCBElementBool: return sizeof(BOOL);
	case kCBElementInt: return sizeof(int);
	case kCBElementInt2: return sizeof(int) * 2;
	case kCBElementInt3: return sizeof(int) * 3;
	case kCBElementInt4: return sizeof(int) * 4;
	case kCBElementFloat: return sizeof(float);
	case kCBElementFloat2: return sizeof(float) * 2;
	case kCBElementFloat3: return sizeof(float) * 3;
	case kCBElementFloat4: return sizeof(float) * 4;
	case kCBElementMatrix: return sizeof(float) * 16;
	case kCBElementMax:
	default:
		BOOST_ASSERT(false);
		break;
	}
	return 0;
}

}