#pragma once
#include "core/base/std.h"

namespace mir {

typedef std::shared_ptr<struct CameraBase> TCameraBasePtr;
typedef std::shared_ptr<struct Camera> TCameraPtr;
typedef std::shared_ptr<struct ISceneManager> ISceneManagerPtr;
typedef std::shared_ptr<struct SceneManager> TSceneManagerPtr;

}