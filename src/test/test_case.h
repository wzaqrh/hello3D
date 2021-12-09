#pragma once
#include "core/mir.h"

namespace test1 {

namespace vec {
inline Eigen::Vector3f DirLight() { return Eigen::Vector3f(0, 0, 10); }
inline Eigen::Vector3f PosLight() { return Eigen::Vector3f(300, 300, 0); }
}

namespace cam {
inline Eigen::Vector3f Eye() { return Eigen::Vector3f(0, 0, -1000); }
inline Eigen::Vector3f NearFarFov() { return Eigen::Vector3f(0.01, 3000, 45); }
}
namespace cam1 {
inline Eigen::Vector3f Eye() { return Eigen::Vector3f(0, 0, -30); }
inline Eigen::Vector3f NearFarFov() { return Eigen::Vector3f(0.01, 300, 45); }
}

namespace res {
inline std::string Sky() { return "model/uffizi_cross.dds"; }

namespace png {
inline std::string Kenny() { return "model/theyKilledKenny.png"; }
}
namespace hdr {
inline std::string Kenny() { return "model/theyKilledKenny.hdr"; }
}
namespace dds {
inline std::string Kenny() { return "model/theyKilledKenny.dds"; }
inline std::string Lenna() { return "model/lenna.dds"; }
}

namespace model_mir {
inline std::string Path() { return "model/Male03/Male02.FBX"; }
inline std::string Rd() { return R"({"ext":"png","dir":"model/Male03/"})"; }
inline Eigen::Vector3f Scale() { return Eigen::Vector3f(0.07, 0.07, 0.07); }
}

namespace model_sship {
inline std::string Path() { return "model/Spaceship/Spaceship.fbx"; }
inline std::string Rd() { return R"({"dir":"model/Spaceship/"})"; }
inline Eigen::Vector3f Scale() { return Eigen::Vector3f(1, 1, 1); }
}

namespace model_rock {
inline std::string Path() { return "model/rock/rock.obj"; }
inline std::string Rd() { return R"({"dir":"model/rock/"})"; }
inline Eigen::Vector3f Scale() { return Eigen::Vector3f(100, 100, 100); }
}
}

}
