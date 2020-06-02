#include "TInterfaceType.h"
#include "IRenderSystem.h"
#include "TInterfaceType11.h"

template<class T>
static IUnknown*& MakeDeviceObjectRef(T*& ref) {
	IUnknown** ppDeviceObj = (IUnknown**)&ref;
	return *ppDeviceObj;
}
/********** TBlobDataStd **********/
TBlobDataStd::TBlobDataStd(const std::vector<char>& buffer)
	:mBuffer(buffer)
{
}

char* TBlobDataStd::GetBufferPointer()
{
	return mBuffer.empty() ? nullptr : &mBuffer[0];
}

size_t TBlobDataStd::GetBufferSize()
{
	return mBuffer.size();
}

/********** IIndexBuffer **********/
int IIndexBuffer::GetCount()
{
	return GetBufferSize() / GetWidth();
}