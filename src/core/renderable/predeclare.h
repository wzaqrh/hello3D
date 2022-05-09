#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr; typedef std::weak_ptr<TYPE> TYPE##WeakPtr;  
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr; typedef std::weak_ptr<TYPE> TYPE##WeakPtr;

DECLARE_STRUCT(TextureVector);

namespace rend {
DECLARE_CLASS(SkyBox);
DECLARE_CLASS(Sprite);
DECLARE_CLASS(Mesh);
DECLARE_CLASS(AssimpModel);
DECLARE_CLASS(Cube);
DECLARE_CLASS(PostProcess);
DECLARE_CLASS(Bloom);
DECLARE_CLASS(Label);
DECLARE_CLASS(Paint3D);
DECLARE_CLASS(LinePaint3D);
}
DECLARE_CLASS(FontCache);

DECLARE_STRUCT(RenderOperation);
DECLARE_STRUCT(RenderOperationQueue);
DECLARE_STRUCT(Renderable);
DECLARE_CLASS(RenderableFactory);

}