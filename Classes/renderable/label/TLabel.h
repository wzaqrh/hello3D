#pragma once
#include "TPredefine.h"
#include "IRenderable.h"
#include "TFont.h"

class TLabel : public IRenderable {
	IRenderSystem* mRenderSys = nullptr;
	TFontPtr mFont;
public:
	TMaterialPtr Material;
	TTransformPtr Transform;
public:
	TLabel(IRenderSystem* renderSys, TFontPtr font);
	virtual int GenRenderOperation(TRenderOperationQueue& opList) override;
};
typedef std::shared_ptr<TLabel> TLabelPtr;