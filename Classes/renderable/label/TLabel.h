#pragma once
//INCLUDE_PREDEFINE_H
#include "IRenderable.h"
#include "TFont.h"
#include "TSprite.h"

class TLabel : public IRenderable {
	IRenderSystem* mRenderSys = nullptr;
	TFontPtr mFont;

	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	TMaterialPtr Material;
	TTransformPtr Transform;
public:
	enum { MAX_STRING_LENGTH = 256 };
private:
	std::string mString;
	struct CharEntry {
		Quad quad;
		TFontCharactorPtr charInfo;
		ITexturePtr texture;
	};
	std::vector<CharEntry> mCharSeq;
	std::vector<int> mCharSeqOrder;
	XMINT2 mMin,mMax;
public:
	TLabel(IRenderSystem* renderSys, TFontPtr font);
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
public:
	void SetString(const std::string& str);
private:
	void UpdateContentSize();
};
typedef std::shared_ptr<TLabel> TLabelPtr;