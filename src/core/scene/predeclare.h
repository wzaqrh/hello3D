#pragma once
#include "core/base/stl.h"

namespace mir {

#define DECLARE_STRUCT(TYPE) struct TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;
#define DECLARE_CLASS(TYPE) class TYPE; typedef std::shared_ptr<TYPE> TYPE##Ptr; typedef TYPE* TYPE##RawPtr;

DECLARE_STRUCT(cbDirectLight);
DECLARE_STRUCT(cbPointLight);
DECLARE_STRUCT(cbSpotLight);
DECLARE_STRUCT(Light);

DECLARE_STRUCT(Camera);
DECLARE_STRUCT(SceneManager);

}