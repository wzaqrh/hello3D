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
float Near();
float Far();
}
namespace cam_otho {
Eigen::Vector3f Eye(Eigen::Vector3f mWinCenter);
float Near();
float Far();
}

namespace res {

std::string Sky(int index = 0);

namespace cube {
namespace far_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname = "");
}
namespace near_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname = "");
}
namespace floor {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, float y, const mir::MaterialLoadParam& matname = "");
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

struct model {
	model();
	model(const std::string& name);
	void Init(const std::string& name);
	mir::TransformPtr Init(const std::string& name, mir::AssimpModelPtr model);
	std::string Path() const { return mPath; }
	std::string Rd() const { return mRd; }
	Eigen::Vector3f Scale() const { return mScale; }
	Eigen::Vector3f Pos() const { return mPos; }
public:
	std::string mName;
	std::string mPath, mRd;
	Eigen::Vector3f mScale, mPos;
};

namespace model_nanosuit {
std::string Path();
std::string Rd();
Eigen::Vector3f Scale();
Eigen::Vector3f Pos();
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f winCenter);
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

namespace model_floor {
std::string Path();
std::string Rd();
Eigen::Vector3f Scale();
Eigen::Vector3f Pos();
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f mWinCenter);
}

namespace model_planet {
std::string Path();
std::string Rd();
Eigen::Vector3f Scale();
Eigen::Vector3f Pos();
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f mWinCenter);
}
}

}
