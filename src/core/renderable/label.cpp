#include "freetype2/freetype/ftglyph.h"
#include "core/base/transform.h"
#include "core/renderable/font.h"
#include "core/renderable/label.h"
#include "core/rendersys/resource_manager.h"
#include "core/rendersys/material_factory.h"
#include "core/rendersys/interface_type.h"

namespace mir {

constexpr int CMaxStringLength = 256;

struct IndicesData {
	unsigned int Indices[CMaxStringLength * 6];
	IndicesData() {
		unsigned int c_indices[6] = { 0, 1, 2, 0, 2, 3 };
		for (size_t i = 0; i < CMaxStringLength; ++i) {
			int base = i * 6, off = i * 4;
			Indices[base + 0] = c_indices[0] + off;
			Indices[base + 1] = c_indices[1] + off;
			Indices[base + 2] = c_indices[2] + off;
			Indices[base + 3] = c_indices[3] + off;
			Indices[base + 4] = c_indices[4] + off;
			Indices[base + 5] = c_indices[5] + off;
		}
	}
};
static IndicesData sIndiceData;

Label::Label(ResourceManager& resourceMng, MaterialFactory& matFac, FontPtr font)
	:mResourceMng(resourceMng)
{
	mFont = font;

	mTransform = std::make_shared<Transform>();
	mMaterial = matFac.GetMaterial(E_MAT_LABEL);

	mIndexBuffer = mResourceMng.CreateIndexBuffer(sizeof(unsigned int) * 6 * CMaxStringLength, kFormatR32UInt, (void*)&sIndiceData.Indices[0]);
	mVertexBuffer = mResourceMng.CreateVertexBuffer(sizeof(SpriteVertexQuad) * CMaxStringLength, sizeof(SpriteVertex), 0, nullptr);
}

int Label::GenRenderOperation(RenderOperationQueue& opList)
{
	if (!mMaterial->IsLoaded()
		|| !mVertexBuffer->IsLoaded()
		|| !mIndexBuffer->IsLoaded())
		return 0;

	int count = 0;
	ITexturePtr lastTex;
	int lastCount = 0;
	for (int i = 0; i < mCharSeqOrder.size(); ++i) {
		int idx = mCharSeqOrder[i];
		assert(idx >= 0 && idx < mCharSeq.size());

		auto& entry = mCharSeq[idx];
		if (entry.texture != lastTex) {
			if (lastTex != nullptr) {
				RenderOperation op = {};
				op.mMaterial = mMaterial;
				op.mVertexBuffer = mVertexBuffer;
				op.mIndexBuffer = mIndexBuffer;
				op.mIndexBase = 4 * (i - lastCount);
				op.mIndexCount = 6 * lastCount;
				op.mTextures.Add(lastTex);
				op.mWorldTransform = mTransform->GetMatrix();
				opList.AddOP(op);
				++count;
			}
			lastTex = entry.texture;
			lastCount = 0;
		}
		++lastCount;
	}

	if (lastTex) {
		RenderOperation op = {};
		op.mMaterial = mMaterial;
		op.mVertexBuffer = mVertexBuffer;
		op.mIndexBuffer = mIndexBuffer;
		op.mIndexBase = 4 * (mCharSeqOrder.size() - lastCount);
		op.mIndexCount = 6 * lastCount;
		op.mTextures.Add(lastTex);
		op.mWorldTransform = mTransform->GetMatrix();
		opList.AddOP(op);
		++count;
	}

	return count;
}

void Label::SetSize(bool autoSize, const Eigen::Vector2f& size)
{
	mAutoUptSize = autoSize;
	if (!autoSize) {
		mConetentSize = size;
	}
	else {
		mConetentSize.x() = mBBox.xMax - mBBox.xMin;
		mConetentSize.y() = mBBox.yMax - mBBox.yMin;
	}

	ForceLayout();
}

void Label::AutoUpdateSize()
{
	if (mAutoUptSize) {
		mConetentSize.x() = mBBox.xMax - mBBox.xMin;
		mConetentSize.y() = mBBox.yMax - mBBox.yMin;
	}
}

inline int Convert266ToDotSpace(int val) {
	int res = val >> 6;
	if ((res << 6) != val) {
		res += val > 0 ? 1 : -1;
	}
	return res;
}
#define TODOT(V) Convert266ToDotSpace(V)

void Label::UpdateBBox()
{
	mLineCount = 1;
	mBBox.xMin = mBBox.yMin = 32000;
	mBBox.xMax = mBBox.yMax = -32000;

	bool useKerning = FT_HAS_KERNING(mFont->GetFtFace());
	int previousCharIndex = 0;

	Eigen::Vector2i pen = { 0, 0 };//dot space
	for (int i = 0; i < mCharSeq.size(); i++)
	{
		mCharSeq[i].pen = pen;
		int c = mString[i];
		if (c == '\n') {
			++mLineCount;
			pen.y() -= TODOT(mFont->GetFtFace()->size->metrics.height);
			pen.x() = 0;
			previousCharIndex = 0;
		}
		else {
			auto& ch = *mCharSeq[i].charInfo;

			if (useKerning && previousCharIndex && ch.CharIndex)
			{
				FT_Vector delta;
				FT_Get_Kerning(mFont->GetFtFace(), previousCharIndex, ch.CharIndex, FT_KERNING_DEFAULT, &delta);
				pen.x() += delta.x >> 6;
			}

			FT_BBox glyph_bbox = ch.BBox;
			glyph_bbox.xMin += pen.x();
			glyph_bbox.xMax += pen.x();
			glyph_bbox.yMin += pen.y();
			glyph_bbox.yMax += pen.y();

			mBBox.xMin = min(mBBox.xMin, (glyph_bbox.xMin));
			mBBox.yMin = min(mBBox.yMin, (glyph_bbox.yMin));
			mBBox.xMax = max(mBBox.xMax, (glyph_bbox.xMax));
			mBBox.yMax = max(mBBox.yMax, (glyph_bbox.yMax));

			mCharSeq[i].quad.SetTexCoord(ch.Atlas->Uv0, ch.Atlas->Uv1);
			mCharSeq[i].quad.FlipY();

			pen.x() += ch.Advance;
			previousCharIndex = ch.CharIndex;
		}
	}

	if (mBBox.xMin > mBBox.xMax) mBBox = { 0,0,0,0 };
}

void Label::ForceLayout()
{
	Eigen::Vector2d stringSize(mBBox.xMax - mBBox.xMin, mBBox.yMax - mBBox.yMin);

	Eigen::Vector2d start(//lt
		(mConetentSize.x() - stringSize.x()) / 2,
		(mConetentSize.y() - stringSize.y()) / 2);

	int startPenY = TODOT(mFont->GetFtFace()->size->metrics.height) * (mLineCount - 1);//1/64 space
	for (int i = 0; i < mCharSeq.size(); i++) {
		if (mString[i] != '\n') {
			auto& pen = mCharSeq[i].pen;
			auto& ch = *mCharSeq[i].charInfo;

			Eigen::Vector2d pos(//lb
				start.x() + (pen.x() + ch.Bearing.x() * mScale),
				start.y() + (pen.y() + startPenY - (ch.Size.y() - ch.Bearing.y()) * mScale));

			auto& quad = mCharSeq[i].quad;
			quad.SetRect(pos.x(), pos.y(), (ch.BBox.xMax - ch.BBox.xMin), (ch.BBox.yMax - ch.BBox.yMin));
		}
	}

	if (mCharSeq.size() > 0) 
	{
		std::vector<SpriteVertexQuad> quadArray;
		for (int i = 0; i < mCharSeqOrder.size(); ++i) {
			int idx = mCharSeqOrder[i];
			assert(idx >= 0 && idx < mCharSeq.size());

			auto& quad = mCharSeq[idx].quad;
			quadArray.push_back(quad);
		}

		mResourceMng.UpdateBuffer(mVertexBuffer, &quadArray[0], quadArray.size() * sizeof(SpriteVertexQuad));
	}
}

void Label::SetString(const std::string& str)
{
	if (mString == str) return;
	mString = str;
	mCharSeq.clear();

	for (int i = 0; i < mString.size(); ++i) {
		int c = mString[i];
		if (c == '\n') {
			mCharSeq.push_back(CharEntry());
		}
		else {
			CharEntry entry;
			entry.charInfo = mFont->GetCharactor(c);
			entry.texture = entry.charInfo->Atlas->Texture;
			mCharSeq.push_back(entry);

			mCharSeqOrder.push_back(i);
		}
	}
	mFont->Flush();

	std::sort(mCharSeqOrder.begin(), mCharSeqOrder.end(), [&](int l, int r) {
		auto& chl = mCharSeq[l];
		auto& chr = mCharSeq[r];
		return chl.texture < chr.texture;
	});

	UpdateBBox();

	AutoUpdateSize();
	ForceLayout();
}

}