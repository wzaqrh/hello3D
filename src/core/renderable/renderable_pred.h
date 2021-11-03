#pragma once
#include "core/base/std.h"

typedef std::shared_ptr<struct TTransform> TTransformPtr;

struct TTextureBySlot;
typedef std::shared_ptr<TTextureBySlot> TTextureBySlotPtr;

struct TRenderOperation;
struct TRenderOperationQueue;

struct IRenderable;

typedef std::shared_ptr<class TSkyBox> TSkyBoxPtr;
typedef std::shared_ptr<class TPostProcess> TPostProcessPtr;
typedef std::shared_ptr<class TSprite> TSpritePtr;
typedef std::shared_ptr<class TMesh> TMeshPtr;
typedef std::shared_ptr<class TLabel> TLabelPtr;
typedef std::shared_ptr<class TFontCache> TFontCachePtr;