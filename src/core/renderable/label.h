#pragma once
#include "core/mir_export.h"
#include "core/base/declare_macros.h"
#include "core/base/launch.h"
#include "core/base/attribute_struct.h"
#include "core/renderable/renderable_base.h"

namespace mir {

typedef std::shared_ptr<class Font> FontPtr;
typedef std::shared_ptr<struct FontCharactor> FontCharactorPtr;

class MIR_CORE_API Label : public RenderableSingleRenderOp 
{
	typedef RenderableSingleRenderOp Super;
	friend class RenderableFactory;
	DECLARE_STATIC_CREATE_CONSTRUCTOR(Label);
	Label(Launch launchMode, ResourceManager& resourceMng, const MaterialLoadParam& matName, FontPtr font);
public:
	void SetString(const std::string& str);
	void SetSize(bool autoCalSize, const Eigen::Vector2f& size);
public:
	const Eigen::Vector2f& GetSize() const { return mConetentSize; }
private:
	void GenRenderOperation(RenderOperationQueue& opList) override;
	void AutoUpdateSize();
	void UpdateBBox();
	void ForceLayout();
private:
	FontPtr mFont;

	std::string mString;
	struct CharEntry {
		vbSurfaceQuad quad;
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
};

}