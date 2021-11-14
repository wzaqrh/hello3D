#pragma once
#include "core/mir_export.h"
#include "core/renderable/renderable.h"
#include "core/renderable/sprite.h"

namespace mir {

typedef std::shared_ptr<class Font> FontPtr;
typedef std::shared_ptr<struct FontCharactor> FontCharactorPtr;

class MIR_CORE_API Label : public IRenderable 
{
	IRenderSystem& mRenderSys;
	FontPtr mFont;

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
	struct BBox {
		long xMin, yMin;
		long xMax, yMax;
	};
	std::vector<CharEntry> mCharSeq;
	std::vector<int> mCharSeqOrder;
	BBox mBBox;//dot space
	int mLineCount = 0;
	Eigen::Vector2f mConetentSize;//dot space
	bool mAutoUptSize = true;
	float mScale = 1;
public:
	Label(IRenderSystem& renderSys, MaterialFactory& matFac, FontPtr font);
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