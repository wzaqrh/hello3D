#pragma once
//INCLUDE_PREDEFINE_H
#include "core/renderable/renderable.h"
#include "core/renderable/font.h"
#include "core/renderable/sprite.h"

namespace mir {

class Label : public IRenderable {
	IRenderSystem* mRenderSys = nullptr;
	TFontPtr mFont;

	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr Material;
public:
	TransformPtr Transform;
	enum { MAX_STRING_LENGTH = 256 };
public:
	std::string mString;
	struct CharEntry {
		Quad quad;
		FontCharactorPtr charInfo;
		ITexturePtr texture;
		XMINT2 pen;//dot space
	};
	std::vector<CharEntry> mCharSeq;
	std::vector<int> mCharSeqOrder;
	FT_BBox mBBox;//dot space
	int mLineCount = 0;
	XMFLOAT2 mConetentSize;//dot space
	bool mAutoUptSize = true;
	float mScale = 1;
public:
	Label(IRenderSystem* renderSys, TFontPtr font);
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
public:
	void SetString(const std::string& str);
	void SetSize(bool autoCalSize, XMFLOAT2 size);
	XMFLOAT2 GetSize() const { return mConetentSize; }
private:
	void AutoUpdateSize();
	void UpdateBBox();
	void ForceLayout();
};
typedef std::shared_ptr<Label> LabelPtr;

}