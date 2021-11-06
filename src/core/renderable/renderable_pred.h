#pragma once
#include "core/base/std.h"

namespace mir {

typedef std::shared_ptr<struct Transform> TransformPtr;

struct TextureBySlot;
typedef std::shared_ptr<TextureBySlot> TTextureBySlotPtr;

struct RenderOperation;
struct RenderOperationQueue;

struct IRenderable;

typedef std::shared_ptr<class SkyBox> SkyBoxPtr;
typedef std::shared_ptr<class PostProcess> TPostProcessPtr;
typedef std::shared_ptr<class Sprite> SpritePtr;
typedef std::shared_ptr<class Mesh> MeshPtr;
typedef std::shared_ptr<class Label> LabelPtr;
typedef std::shared_ptr<class FontCache> FontCachePtr;

}