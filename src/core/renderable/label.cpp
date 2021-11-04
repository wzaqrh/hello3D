#include "core/renderable/label.h"
#include "core/base/transform.h"
#include "core/rendersys/render_system.h"
#include "core/rendersys/material.h"
#include "core/rendersys/interface_type.h"
#include "freetype2/freetype/ftglyph.h"

namespace mir {

struct IndicesData {
	unsigned int Indices[TLabel::MAX_STRING_LENGTH * 6];
	IndicesData() {
		unsigned int c_indices[6] = { 0, 1, 2, 0, 2, 3 };
		for (size_t i = 0; i < TLabel::MAX_STRING_LENGTH; ++i) {
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

TLabel::TLabel(IRenderSystem* renderSys, TFontPtr font)
{
	mRenderSys = renderSys;
	mFont = font;

	Transform = std::make_shared<TMovable>();
	Material = renderSys->CreateMaterial(E_MAT_LABEL, nullptr);

	mIndexBuffer = mRenderSys->CreateIndexBuffer(sizeof(unsigned int) * 6 * MAX_STRING_LENGTH, DXGI_FORMAT_R32_UINT, (void*)&sIndiceData.Indices[0]);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(Quad) * MAX_STRING_LENGTH, sizeof(Pos3Color3Tex2), 0);
}

int TLabel::GenRenderOperation(TRenderOperationQueue& opList)
{
	int count = 0;
	ITexturePtr lastTex;
	int lastCount = 0;
	for (int i = 0; i < mCharSeqOrder.size(); ++i) {
		int idx = mCharSeqOrder[i];
		assert(idx >= 0 && idx < mCharSeq.size());

		auto& entry = mCharSeq[idx];
		if (entry.texture != lastTex) {
			if (lastTex != nullptr) {
				TRenderOperation op = {};
				op.mMaterial = Material;
				op.mVertexBuffer = mVertexBuffer;
				op.mIndexBuffer = mIndexBuffer;
				op.mIndexBase = 4 * (i - lastCount);
				op.mIndexCount = 6 * lastCount;
				op.mTextures.push_back(lastTex);
				op.mWorldTransform = Transform->Matrix();
				opList.AddOP(op);
				++count;
			}
			lastTex = entry.texture;
			lastCount = 0;
		}
		++lastCount;
	}

	if (lastTex) {
		TRenderOperation op = {};
		op.mMaterial = Material;
		op.mVertexBuffer = mVertexBuffer;
		op.mIndexBuffer = mIndexBuffer;
		op.mIndexBase = 4 * (mCharSeqOrder.size() - lastCount);
		op.mIndexCount = 6 * lastCount;
		op.mTextures.push_back(lastTex);
		op.mWorldTransform = Transform->Matrix();
		opList.AddOP(op);
		++count;
	}

	return count;
}

void TLabel::SetSize(bool autoSize, XMFLOAT2 size)
{
	mAutoUptSize = autoSize;
	if (!autoSize) {
		mConetentSize = size;
	}
	else {
		mConetentSize.x = mBBox.xMax - mBBox.xMin;
		mConetentSize.y = mBBox.yMax - mBBox.yMin;
	}

	ForceLayout();
}

void TLabel::AutoUpdateSize()
{
	if (mAutoUptSize) {
		mConetentSize.x = mBBox.xMax - mBBox.xMin;
		mConetentSize.y = mBBox.yMax - mBBox.yMin;
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

void TLabel::UpdateBBox()
{
	mLineCount = 1;
	mBBox.xMin = mBBox.yMin = 32000;
	mBBox.xMax = mBBox.yMax = -32000;

	bool useKerning = FT_HAS_KERNING(mFont->GetFtFace());
	int previousCharIndex = 0;

	XMINT2 pen = { 0, 0 };//dot space
	for (int i = 0; i < mCharSeq.size(); i++)
	{
		mCharSeq[i].pen = pen;
		int c = mString[i];
		if (c == '\n') {
			++mLineCount;
			pen.y -= TODOT(mFont->GetFtFace()->size->metrics.height);
			pen.x = 0;
			previousCharIndex = 0;
		}
		else {
			auto& ch = *mCharSeq[i].charInfo;

			if (useKerning && previousCharIndex && ch.charIndex)
			{
				FT_Vector delta;
				FT_Get_Kerning(mFont->GetFtFace(), previousCharIndex, ch.charIndex, FT_KERNING_DEFAULT, &delta);
				pen.x += delta.x >> 6;
			}

			FT_BBox glyph_bbox = ch.bbox;
			glyph_bbox.xMin += pen.x;
			glyph_bbox.xMax += pen.x;
			glyph_bbox.yMin += pen.y;
			glyph_bbox.yMax += pen.y;

			mBBox.xMin = min(mBBox.xMin, (glyph_bbox.xMin));
			mBBox.yMin = min(mBBox.yMin, (glyph_bbox.yMin));
			mBBox.xMax = max(mBBox.xMax, (glyph_bbox.xMax));
			mBBox.yMax = max(mBBox.yMax, (glyph_bbox.yMax));

			mCharSeq[i].quad.SetTexCoord(ch.Atlas->UV0, ch.Atlas->UV1);
			mCharSeq[i].quad.FlipY();

			pen.x += ch.Advance;
			previousCharIndex = ch.charIndex;
		}
	}

	if (mBBox.xMin > mBBox.xMax) mBBox = { 0,0,0,0 };
}

void TLabel::ForceLayout()
{
	XMINT2 stringSize = { mBBox.xMax - mBBox.xMin, mBBox.yMax - mBBox.yMin };

	XMINT2 start;//lt
	start.x = (mConetentSize.x - stringSize.x) / 2;
	start.y = (mConetentSize.y - stringSize.y) / 2;

	int startPenY = TODOT(mFont->GetFtFace()->size->metrics.height) * (mLineCount - 1);//1/64 space
	for (int i = 0; i < mCharSeq.size(); i++) {
		if (mString[i] != '\n') {
			auto& pen = mCharSeq[i].pen;
			auto& ch = *mCharSeq[i].charInfo;

			XMINT2 pos;//lb
			pos.x = start.x + (pen.x + ch.Bearing.x * mScale);
			pos.y = start.y + (pen.y + startPenY - (ch.Size.y - ch.Bearing.y) * mScale);

			auto& quad = mCharSeq[i].quad;
			quad.SetRect(pos.x, pos.y, (ch.bbox.xMax - ch.bbox.xMin), (ch.bbox.yMax - ch.bbox.yMin));
		}
	}

	if (mCharSeq.size() > 0) 
	{
		std::vector<Quad> quadArray;
		for (int i = 0; i < mCharSeqOrder.size(); ++i) {
			int idx = mCharSeqOrder[i];
			assert(idx >= 0 && idx < mCharSeq.size());

			auto& quad = mCharSeq[idx].quad;
			quadArray.push_back(quad);
		}

		mRenderSys->UpdateBuffer((mVertexBuffer), &quadArray[0], quadArray.size() * sizeof(Quad));
	}
}

void TLabel::SetString(const std::string& str)
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