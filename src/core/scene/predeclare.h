#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

namespace scene {
DECLARE_STRUCT(ILight);
DECLARE_CLASS(DirectLight);
DECLARE_CLASS(PointLight);
DECLARE_CLASS(SpotLight);
DECLARE_CLASS(Camera);
}
DECLARE_CLASS(SceneManager);

}