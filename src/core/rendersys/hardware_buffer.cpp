#include "core/rendersys/hardware_buffer.h"

namespace mir {

size_t CbDeclElement::GetByteWidth(Type type)
{
	switch (type)
	{
	case Type::Bool: return sizeof(BOOL);
	case Type::Int: return sizeof(int);
	case Type::Int2: return sizeof(int) * 2;
	case Type::Int3: return sizeof(int) * 3;
	case Type::Int4: return sizeof(int) * 4;
	case Type::Float: return sizeof(float);
	case Type::Float2: return sizeof(float) * 2;
	case Type::Float3: return sizeof(float) * 3;
	case Type::Float4: return sizeof(float) * 4;
	case Type::Matrix: return sizeof(float) * 16;
	case Type::Max:
	default:
		BOOST_ASSERT(false);
		break;
	}
	return 0;
}

size_t CbDeclElement::GetByteWidth() const
{
	return GetByteWidth(Type1);
}

}