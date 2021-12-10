#include <boost/math/constants/constants.hpp>
#include "test/test_case.h"
#include "core/base/transform.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"

namespace test1 {

namespace math {
float ToRadian(float angle) { return angle / boost::math::constants::radian<float>(); }
}

namespace vec {
Eigen::Vector3f DirLight() { return Eigen::Vector3f(0, 0, 10); }
Eigen::Vector3f PosLight() { return Eigen::Vector3f(30, 30, -30); }
}

namespace cam {
constexpr float EyeZ = -10;
Eigen::Vector3f Eye(Eigen::Vector3f mWinCenter) { return Eigen::Vector3f(0, 0, EyeZ); }
Eigen::Vector3f NearFarFov() { return Eigen::Vector3f(0.3, 1000, 60); }
float Near() { return EyeZ + NearFarFov().x(); }
float Far() { return EyeZ + NearFarFov().y(); }
}
namespace cam_otho {
constexpr float EyeZ = -10;
Eigen::Vector3f Eye(Eigen::Vector3f winCenter) { return Eigen::Vector3f(0, 0, EyeZ); }
Eigen::Vector3f NearFarFov() { return Eigen::Vector3f(0.3, 1000, 60); }
float Near() { return EyeZ + NearFarFov().x() + 1; }
float Far() { return EyeZ + NearFarFov().y() - 1; }
}

namespace res {
std::string Sky() { return "model/uffizi_cross.dds"; }

namespace cube {
namespace far_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter) {
	constexpr int SizeBig = 8192;
	return rendFac->CreateCube(
		winCenter + Eigen::Vector3f(0, 0, 300),
		Eigen::Vector3f(SizeBig, SizeBig, 1),
		0xffff6347
	);
}
}
namespace near_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter) {
	constexpr int SizeSmall = 4;
	return rendFac->CreateCube(
		winCenter + Eigen::Vector3f(-SizeSmall, -SizeSmall, test1::cam::Near()),
		Eigen::Vector3f(SizeSmall * 2, SizeSmall * 2, 1),
		0xffff4763
	);
}
}
}

namespace png {
std::string Kenny() { return "model/theyKilledKenny.png"; }
void SetPos(mir::SpritePtr sprite, Eigen::Vector3f pos, Eigen::Vector3f size, Eigen::Vector3f anchor) {
	sprite->SetPosition(pos);
	sprite->SetSize(size);
	sprite->SetAnchor(anchor);
}
}
namespace hdr {
std::string Kenny() { return "model/theyKilledKenny.hdr"; }
}
namespace dds {
std::string Kenny() { return "model/theyKilledKenny.dds"; }
std::string Lenna() { return "model/lenna.dds"; }
}

namespace model_mir {
std::string Path() { return "model/Male03/Male02.FBX"; }
std::string Rd() { return R"({"ext":"png","dir":"model/Male03/"})"; }
constexpr float scale = 0.05;
Eigen::Vector3f Scale() { return Eigen::Vector3f(scale, scale, scale); }
Eigen::Vector3f Pos() { return Eigen::Vector3f(0, -5, 0); }
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f mWinCenter) {
	model->LoadModel(Path(), Rd());
	mir::TransformPtr transform = model->GetTransform();
	transform->SetScale(Scale());
	transform->SetPosition(mWinCenter + Pos());
	return transform;
}
}

namespace model_sship {
std::string Path() { return "model/Spaceship/Spaceship.fbx"; }
std::string Rd() { return R"({"dir":"model/Spaceship/"})"; }
constexpr float scale = 0.02;
Eigen::Vector3f Scale() { return Eigen::Vector3f(scale, scale, scale); }
Eigen::Vector3f Pos() { return Eigen::Vector3f(0, 0, 0); }
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f mWinCenter) {
	model->LoadModel(Path(), Rd());
	mir::TransformPtr transform = model->GetTransform();
	transform->SetScale(Scale());
	transform->SetPosition(mWinCenter + Pos());
	return transform;
}
}

namespace model_rock {
std::string Path() { return "model/rock/rock.obj"; }
std::string Rd() { return R"({"dir":"model/rock/"})"; }
Eigen::Vector3f Scale() { return Eigen::Vector3f(1, 1, 1); }
Eigen::Vector3f Pos() { return Eigen::Vector3f(0, 0, 0); }
mir::TransformPtr Init(mir::AssimpModelPtr model, Eigen::Vector3f mWinCenter) {
	model->LoadModel(Path(), Rd());
	mir::TransformPtr transform = model->GetTransform();
	transform->SetScale(Scale());
	transform->SetPosition(mWinCenter + Pos());
	return transform;
}
}

}
}
