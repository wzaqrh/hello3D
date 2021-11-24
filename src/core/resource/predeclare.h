#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

DECLARE_STRUCT(IResource);
DECLARE_STRUCT(Pass);
DECLARE_STRUCT(Technique);
DECLARE_STRUCT(Material);
DECLARE_STRUCT(MaterialFactory);

DECLARE_CLASS(AiNode);
DECLARE_CLASS(AiScene);
DECLARE_CLASS(AiResourceFactory);
DECLARE_CLASS(ResourceManager);

}