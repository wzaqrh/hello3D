#include "TLabel.h"
#include "TTransform.h"
#include "IRenderSystem.h"
#include "TMaterial.h"
#include "TInterfaceType.h"

struct IndicesData {
	unsigned int Indices[TLabel::MAX_STRING_LENGTH * 6];
	IndicesData() {
		unsigned int c_indices[6] = { 0, 1, 2, 0, 2, 3 };
		for (size_t i = 0; i < TLabel::MAX_STRING_LENGTH; ++i)
			memcpy(Indices + i * 6, c_indices, sizeof(c_indices));
	}
};
static IndicesData sIndiceData;

TLabel::TLabel(IRenderSystem* renderSys, TFontPtr font)
{
	mRenderSys = renderSys;
	mFont = font;

	Transform = std::make_shared<TMovable>();
	Material = renderSys->CreateMaterial(E_MAT_SPRITE, nullptr);

	mIndexBuffer = mRenderSys->CreateIndexBuffer(sizeof(sIndiceData.Indices), DXGI_FORMAT_R32_UINT, (void*)&sIndiceData.Indices[0]);
	mVertexBuffer = mRenderSys->CreateVertexBuffer(sizeof(Quad), sizeof(Pos3Color3Tex2), 0);
}

int TLabel::GenRenderOperation(TRenderOperationQueue& opList)
{
	TRenderOperation op = {};
	op.mMaterial = Material;
	op.mIndexBuffer = mIndexBuffer;
	op.mVertexBuffer = mVertexBuffer;
	
	op.mWorldTransform = Transform->Matrix();
	opList.AddOP(op);

	return 1;
}

void TLabel::SetString(const std::string& str)
{
	if (mString == str) return;
	mString = str;
	mCharSeq.clear();

	float scale = 1;
	XMINT2 pen = { 0, 0 };//×óÉÏ½Ç
	for (int i = 0; i < mString.size(); ++i) {
		int c = mString[i];
		
		CharEntry entry;
		entry.charInfo = mFont->GetCharactor(c);
		entry.texture = entry.charInfo->Atlas->Texture;
		//auto& ch = *entry.charInfo;
		//entry.quad.SetRect(pen.x + ch.Bearing.x * scale, pen.y - (ch.Size.y - ch.Bearing.y) * scale, ch.Size.x, ch.Size.y);
		mCharSeq.push_back(entry);

		mCharSeqOrder.push_back(i);
	}

	std::sort(mCharSeqOrder.begin(), mCharSeqOrder.end(), [&](int l, int r) {
		auto& chl = mCharSeq[l];
		auto& chr = mCharSeq[r];
		return chl.texture < chr.texture;
	});
}

void TLabel::UpdateContentSize()
{
	mMax = mMin = { 0, 0 };
	float scale = 1;
	XMINT2 pen = { 0, 0 };//×óÉÏ½Ç
	int h = 0;
	for (int i = 0; i < mCharSeq.size(); ++i) {
		int c = mString[i];
		auto& ch = *mCharSeq[i].charInfo;
		
		int xMin = pen.x + ch.Bearing.x * scale; 
		int xMax = xMin + ch.Size.x;
		int yMin = pen.y - (ch.Size.y - ch.Bearing.y) * scale;
		int yMax = yMin + ch.Size.y;

		mMin.x = min(mMin.x, xMin);
		mMin.y = min(mMin.y, yMin);
		mMax.x = max(mMax.x, xMax);
		mMax.y = max(mMax.y, yMax);
		h = 
	}

}
