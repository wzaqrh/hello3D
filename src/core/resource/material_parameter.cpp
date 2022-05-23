#include "core/base/data.h"
#include "core/rendersys/render_system.h"
#include "core/resource/resource_manager.h"
#include "core/resource/material_parameter.h"

namespace mir {
namespace res {

/********** UniformParameters **********/
bool UniformParameters::SetPropertyByString(const std::string& name, std::string strDefault)
{
	int slot = FindProperty(name);
	if (slot >= 0) {
		const auto& decl = mDecl[slot];
		switch (decl.Type1) {
		case CbDeclElement::Type::Bool:
		case CbDeclElement::Type::Int:
		case CbDeclElement::Type::Int2:
		case CbDeclElement::Type::Int3:
		case CbDeclElement::Type::Int4:
			mDataDirty = true;
			mData.SetByParseString<int>(decl.Offset / sizeof(int), decl.Size / sizeof(int), strDefault);
			break;
		case CbDeclElement::Type::Float:
		case CbDeclElement::Type::Float2:
		case CbDeclElement::Type::Float3:
		case CbDeclElement::Type::Float4:
		case CbDeclElement::Type::Matrix:
			mDataDirty = true;
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
	BOOST_ASSERT(!mIsReadOnly);
	BOOST_ASSERT(mDecl.BufferSize >= cbuffer->GetBufferSize());
	renderSys.UpdateBuffer(cbuffer, Data::Make(mData.GetBytes()));
}

/********** UniformParametersBuilder **********/
void UniformParametersBuilder::AddParameter(const std::string& name, CbDeclElement::Type type, size_t size, size_t count, size_t offset,
	const std::string& defValue)
{
	auto& element = mResult.mDecl.Emplace();
	element.Name = name;
	element.Type1 = type;
	element.Size = (size > 0) ? size : CbDeclElement::GetByteWidth(type) * std::max<int>(1, count);
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

/********** GpuUniformsParameters **********/
GpuParameters::Element GpuParameters::Element::Clone(Launch launchMode, ResourceManager& resMng) const
{
	auto cbuffer = Parameters->CreateConstBuffer(launchMode, resMng, CBuffer->GetUsage());
	auto parameters = mir::CreateInstance<UniformParameters>(*Parameters);
	return Element(cbuffer, parameters);
}

GpuParametersPtr GpuParameters::Clone(Launch launchMode, ResourceManager& resMng) const
{
	GpuParametersPtr result = mir::CreateInstance<GpuParameters>();
	for (const auto& element : *this) {
		if (element.IsValid()) {
			result->AddElement(element.Clone(launchMode, resMng));
		}
	}
	return result;
}

void GpuParameters::WriteToElementCb(RenderSystem& renderSys, const std::string& cbName, Data data)
{
	for (const auto& element : *this) {
		if (element.IsValid() && element.GetName() == cbName) {
			renderSys.UpdateBuffer(element.CBuffer, data);
		}
	}
}

void GpuParameters::FlushToGpu(RenderSystem& renderSys)
{
	for (const auto& element : *this) {
		if (element.IsValid() && element.Parameters->IsDataDirty()) {
			element.Parameters->SetDataDirty(false);
			element.Parameters->WriteToConstBuffer(renderSys, element.CBuffer);
		}
	}
}

std::vector<mir::IContantBufferPtr> GpuParameters::GetConstBuffers() const
{
	std::vector<IContantBufferPtr> result(mElements.Count());
	struct ElementToCBuffer {
		IContantBufferPtr operator()(const Element& element) const {
			return element.CBuffer;
		}
	};
	std::transform(mElements.begin(), mElements.end(), result.begin(), ElementToCBuffer());
	return std::move(result);
}

}
}