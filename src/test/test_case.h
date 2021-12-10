#pragma once
#include "core/mir.h"

namespace test1 {

namespace math {
float ToRadian(float angle);
}

namespace vec {
Eigen::Vector3f DirLight();
Eigen::Vector3f PosLight();
}

namespace cam {
Eigen::Vector3f Eye(Eigen::Vector3f mWinCenter);
Eigen::Vector3f NearFarFov();
float Near();
float Far();
void AdjustScale(mir::CameraPtr cam1);
}
namespace cam1 {
Eigen::Vector3f Eye(Eigen::Vector3f mWinCenter);
Eigen::Vector3f NearFarFov();
float Near();
float Far();
}
namespace cam_otho {
Eigen::Vector3f Eye(Eigen::Vector3f mWinCenter);
Eigen::Vector3f NearFarFov();
float Near();
float Far();
}

namespace res {

std::string Sky();

namespace cube {
namespace far_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter);
}
namespace near_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter);
}
}

namespace png {
std::string Kenny();
void SetPos(mir::SpritePtr sp, Eigen::Vector3f pos, Eigen::Vector3f size, Eigen::Vector3f anchor = mir::math::vec::anchor::LeftBottom());
}
namespace hdr {
std::string Kenny();
}
namespace dds {
std::string Kenny();
std::string Lenna();
}

namespace model_mir {
std::string Path();
std::string Rd();
Eigen::Vector3f Scale();
Eigen::Vector3f Pos();
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f winCenter);
}

namespace model_sship {
std::string Path();
std::string Rd();
Eigen::Vector3f Scale();
Eigen::Vector3f Pos();
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f winCenter);
}

namespace model_rock {
std::string Path();
std::string Rd();
Eigen::Vector3f Scale();
Eigen::Vector3f Pos();
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f mWinCenter);
}
}

}
