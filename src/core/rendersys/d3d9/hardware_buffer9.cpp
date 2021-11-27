#include "core/rendersys/d3d9/hardware_buffer9.h"
#include "core/base/d3d.h"

namespace mir {

/********** TIndexBuffer9 **********/
int IndexBuffer9::GetWidth() const
{
	return d3d::BytePerPixel(static_cast<DXGI_FORMAT>(Format));
}

/********** TContantBuffer9 **********/
ContantBuffer9::ContantBuffer9(ConstBufferDeclPtr decl)
	:mDecl(decl)
{
	assert(mDecl != nullptr);
	mBuffer9.resize(mDecl->BufferSize);
}

int ContantBuffer9::GetBufferSize() const
{
	return mDecl->BufferSize;
}

char* ContantBuffer9::GetBuffer9()
{
	return mBuffer9.empty() ? nullptr : &mBuffer9[0];
}

void ContantBuffer9::SetBuffer9(const void* data, int dataSize)
{
	mBuffer9.assign((const char*)data, (const char*)data + dataSize);
}

}