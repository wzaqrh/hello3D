#pragma once
#include "core/base/std.h"

namespace mir {

typedef std::shared_ptr<struct Transform> TransformPtr;
typedef std::shared_ptr<struct TextureBySlot> TTextureBySlotPtr;

typedef std::shared_ptr<class SkyBox> SkyBoxPtr;
typedef std::shared_ptr<class PostProcess> PostProcessPtr;
typedef std::shared_ptr<class Sprite> SpritePtr;
typedef std::shared_ptr<class Mesh> MeshPtr;
typedef std::shared_ptr<class Label> LabelPtr;
typedef std::shared_ptr<class FontCache> FontCachePtr;

}