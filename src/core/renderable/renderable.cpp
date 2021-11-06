#include "core/renderable/renderable.h"
#include "core/rendersys/interface_type.h"

namespace mir {

/********** TTextureBySlot **********/
void TextureBySlot::clear()
{
	textures.clear();
}

void TextureBySlot::push_back(ITexturePtr texture)
{
	textures.push_back(texture);
}

bool TextureBySlot::empty() const
{
	return textures.empty();
}

size_t TextureBySlot::size() const
{
	return textures.size();
}

void TextureBySlot::swap(TextureBySlot& other)
{
	textures.swap(other.textures);
}

void TextureBySlot::resize(size_t size)
{
	textures.resize(size);
}

const ITexturePtr TextureBySlot::At(size_t pos)  const {
	return textures[pos];
}
ITexturePtr& TextureBySlot::At(size_t pos) {
	if (pos >= textures.size()) textures.resize(pos + 1);
	return textures[pos];
}
const ITexturePtr TextureBySlot::operator[](size_t pos)  const {
	return At(pos);
}
ITexturePtr& TextureBySlot::operator[](size_t pos) {
	return At(pos);
}

void TextureBySlot::Merge(const TextureBySlot& other) {
	if (textures.size() < other.textures.size())
		textures.resize(other.textures.size());

	for (size_t i = 0; i < other.textures.size(); ++i) {
		if (other.textures[i] && other.textures[i]->HasSRV()) {
			textures[i] = other.textures[i];
		}
	}
}

/********** TRenderOperation **********/
RenderOperation::RenderOperation()
	: mIndexPos(0)
	, mIndexCount(0)
	, mIndexBase(0)
{
	mWorldTransform = XMMatrixIdentity();
}

/********** TRenderOperationQueue **********/
void RenderOperationQueue::Clear()
{
	mOps.clear();
}

void RenderOperationQueue::AddOP(const RenderOperation& op)
{
	mOps.push_back(op);
}

size_t RenderOperationQueue::Count() const
{
	return mOps.size();
}

const RenderOperation& RenderOperationQueue::At(size_t pos) const
{
	return mOps[pos];
}

RenderOperation& RenderOperationQueue::At(size_t pos)
{
	return mOps[pos];
}

const RenderOperation& RenderOperationQueue::operator[](size_t pos) const
{
	return At(pos);
}

RenderOperation& RenderOperationQueue::operator[](size_t pos)
{
	return At(pos);
}

}