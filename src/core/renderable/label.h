#pragma once
#include "core/renderable/renderable.h"
#include "core/renderable/font.h"
#include "core/renderable/sprite.h"

namespace mir {

class Label : public IRenderable 
{
	IRenderSystem& mRenderSys;
	TFontPtr mFont;

	IVertexBufferPtr mVertexBuffer;
	IIndexBufferPtr mIndexBuffer;
	MaterialPtr mMaterial;
public:
	TransformPtr mTransform;
	enum { MAX_STRING_LENGTH = 256 };
public:
	std::string mString;
	struct CharEntry {
		SpriteVertexQuad quad;
		FontCharactorPtr charInfo;
		ITexturePtr texture;
		Eigen::Vector2i pen;//dot space
	};
	std::vector<CharEntry> mCharSeq;
	std::vector<int> mCharSeqOrder;
	FT_BBox mBBox;//dot space
	int mLineCount = 0;
	Eigen::Vector2f mConetentSize;//dot space
	bool mAutoUptSize = true;
	float mScale = 1;
public:
	Label(IRenderSystem& renderSys, MaterialFactory& matFac, TFontPtr font);
	virtual int GenRenderOperation(RenderOperationQueue& opList) override;
public:
	void SetString(const std::string& str);
	void SetSize(bool autoCalSize, const Eigen::Vector2f& size);
	const Eigen::Vector2f& GetSize() const { return mConetentSize; }
private:
	void AutoUpdateSize();
	void UpdateBBox();
	void ForceLayout();
};

}