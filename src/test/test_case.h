#pragma once
#include "core/mir.h"

using namespace mir;
using namespace mir::rend;
using namespace mir::scene;

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

namespace sky {
namespace footprint_court {
std::string Diffuse();
std::string Specular();
}
}

namespace cube {
namespace far_plane {
CoTask<mir::rend::CubePtr> Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname = "");
}
namespace near_plane {
CoTask<mir::rend::CubePtr> Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname = "");
}
namespace floor {
CoTask<mir::rend::CubePtr> Create(mir::RenderableFactoryPtr rendFac, float y, const mir::MaterialLoadParam& matname = "");
}
}

namespace png {
std::string Kenny();
void SetPos(mir::rend::SpritePtr sp, Eigen::Vector3f pos, Eigen::Vector3f size, Eigen::Vector3f anchor = mir::math::vec::anchor::LeftBottom());
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
	CoTask<mir::TransformPtr> Init(const std::string& name, mir::rend::AssimpModelPtr model);
	std::string Path() const { return mPath; }
	std::string Rd() const { return mRd; }
	Eigen::Vector3f Scale() const { return mScale; }
	Eigen::Vector3f Pos() const { return mPos; }
public:
	std::string mName;
	std::string mPath, mRd;
	Eigen::Vector3f mScale, mPos;
};

}

}
