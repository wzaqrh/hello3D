#include "core/base/data.h"
#include "core/rendersys/render_system.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_parameter.h"

namespace mir {

/********** UniformParameters **********/
bool UniformParameters::SetPropertyByString(const std::string& name, std::string strDefault)
{
	int slot = FindProperty(name);
	if (slot >= 0) {
		const auto& decl = mDecl[slot];
		switch (decl.Type) {
		case kCBElementBool:
		case kCBElementInt:
		case kCBElementInt2:
		case kCBElementInt3:
		case kCBElementInt4:
			mData.SetByParseString<int>(decl.Offset / sizeof(int), decl.Size / sizeof(int), strDefault);
			break;
		case kCBElementFloat:
		case kCBElementFloat2:
		case kCBElementFloat3:
		case kCBElementFloat4:
		case kCBElementMatrix:
			mData.SetByParseString<float>(decl.Offset / sizeof(int), decl.Size / sizeof(int), strDefault);
			break;
		default:
			break;
		}
	}
	return slot >= 0;
}

IContantBufferPtr UniformParameters::CreateConstBuffer(Launch launchMode, ResourceManager& resMng, HWMemoryUsage usage) const
{
	return resMng.CreateConstBuffer(launchMode, mDecl, usage, Data::Make(mData.GetBytes()));
}

void UniformParameters::WriteToConstBuffer(RenderSystem& renderSys, IContantBufferPtr cbuffer) const
{
	BOOST_ASSERT(mDecl.ByteSize() >= cbuffer->GetBufferSize());
	renderSys.UpdateBuffer(cbuffer, Data::Make(mData.GetBytes()));
}

/********** UniformParametersBuilder **********/
void UniformParametersBuilder::AddParameter(const std::string& name, CbElementType type, size_t size, size_t count, size_t offset, 
	const std::string& defValue) 
{
	auto& element = mResult.mDecl.Emplace();
	element.Name = name;
	element.Type = type;
	element.Size = (size > 0) ? size : GetCbElementTypeByteWidth(type) * std::max<int>(1, count);
	element.Count = count;
	element.Offset = (offset >= 0) ? offset : mCurrentByteOffset;
	element.Offset = mResult.mDecl.BufferSize;
	mResult.mDecl.BufferSize += element.Size;

	mCurrentByteOffset = offset + size;

	mResult.mData.Emplaces<float>(element.Size / sizeof(float));
	mResult.SetPropertyByString(name, defValue);
}

UniformParameters& UniformParametersBuilder::Build()
{ 
	int dataSize = mResult.mData.ByteSize();
	if (dataSize & 15) 
		mResult.mData.SetByteSize((dataSize + 15) / 16 * 16);

	return mResult; 
}

}