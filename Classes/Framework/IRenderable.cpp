#include "IRenderable.h"
#include "TInterfaceType.h"

/********** TTextureBySlot **********/
void TTextureBySlot::clear()
{
	textures.clear();
}

void TTextureBySlot::push_back(ITexturePtr texture)
{
	textures.push_back(texture);
}

bool TTextureBySlot::empty() const
{
	return textures.empty();
}

size_t TTextureBySlot::size() const
{
	return textures.size();
}

void TTextureBySlot::swap(TTextureBySlot& other)
{
	textures.swap(other.textures);
}

void TTextureBySlot::resize(size_t size)
{
	textures.resize(size);
}

const ITexturePtr TTextureBySlot::At(size_t pos)  const {
	return textures[pos];
}
ITexturePtr& TTextureBySlot::At(size_t pos) {
	if (pos >= textures.size()) textures.resize(pos + 1);
	return textures[pos];
}
const ITexturePtr TTextureBySlot::operator[](size_t pos)  const {
	return At(pos);
}
ITexturePtr& TTextureBySlot::operator[](size_t pos) {
	return At(pos);
}

std::vector<ID3D11ShaderResourceView*> TTextureBySlot::GetTextureViews11() const {
	std::vector<ID3D11ShaderResourceView*> views(textures.size());
	for (int i = 0; i < views.size(); ++i) {
		if (textures[i]) {
			views[i] = textures[i]->GetSRV11();
		}
	}
	return views;
}

void TTextureBySlot::Merge(const TTextureBySlot& other) {
	if (textures.size() < other.textures.size())
		textures.resize(other.textures.size());

	for (size_t i = 0; i < other.textures.size(); ++i) {
		if (other.textures[i] && other.textures[i]->HasSRV()) {
			textures[i] = other.textures[i];
		}
	}
}

/********** TRenderOperation **********/
TRenderOperation::TRenderOperation()
{
	mWorldTransform = XMMatrixIdentity();
}

/********** TRenderOperationQueue **********/
void TRenderOperationQueue::clear()
{
	mOps.clear();
}

void TRenderOperationQueue::push_back(const TRenderOperation& op)
{
	mOps.push_back(op);
}

size_t TRenderOperationQueue::size() const
{
	return mOps.size();
}

const TRenderOperation& TRenderOperationQueue::At(size_t pos) const
{
	return mOps[pos];
}

TRenderOperation& TRenderOperationQueue::At(size_t pos)
{
	return mOps[pos];
}

const TRenderOperation& TRenderOperationQueue::operator[](size_t pos) const
{
	return At(pos);
}

TRenderOperation& TRenderOperationQueue::operator[](size_t pos)
{
	return At(pos);
}