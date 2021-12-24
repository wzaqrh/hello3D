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
Eigen::Vector3f PosLight() { return Eigen::Vector3f(5, 5, -5); }
}

namespace cam {
constexpr float EyeZ = -10, Diff = 0.5;
Eigen::Vector3f Eye(Eigen::Vector3f mWinCenter) { return Eigen::Vector3f(0, 0, EyeZ); }
float Near() { return EyeZ + 0.3 + Diff; }
float Far() { return EyeZ + 1000 - Diff; }
}
namespace cam_otho {
constexpr float EyeZ = -10, Diff = 0.1;
Eigen::Vector3f Eye(Eigen::Vector3f winCenter) { return Eigen::Vector3f(0, 0, EyeZ); }
float Near() { return EyeZ + 0.3 + Diff; }
float Far() { return EyeZ + 1000 - Diff; }
}

namespace res {
std::string Sky() { return "model/uffizi_cross.dds"; }

namespace cube {
namespace far_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname) {
	constexpr int SizeBig = 8192;
	return rendFac->CreateCube(
		Eigen::Vector3f(0, 0, test1::cam::Far()),
		Eigen::Vector3f(SizeBig, SizeBig, 1),
		0xffff6347,
		matname
	);
}
}
namespace near_plane {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname) {
	constexpr int SizeSmall = 4;
	return rendFac->CreateCube(
		Eigen::Vector3f(0, 0, test1::cam::Near()),
		Eigen::Vector3f(SizeSmall, SizeSmall, 1),
		0xffff4763,
		matname
	);
}
}
namespace floor {
mir::CubePtr Create(mir::RenderableFactoryPtr rendFac, float y, const mir::MaterialLoadParam& matname) {
	constexpr int Inf = 65536;
	return rendFac->CreateCube(
		Eigen::Vector3f(0, y, Inf / 2),
		Eigen::Vector3f(Inf, 1, Inf / 2),
		0xffff4763,
		matname
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

struct ModelInfo {
	std::string mPath, mRd;
	double mScale, mPosY;
};
std::map<std::string, ModelInfo> CResPathMap = {
	{"toycar", {"model/khronos-gltf-pbr/2.0/ToyCar/glTF/ToyCar.gltf","",100,0}},
	{"nanosuit", {"model/nanosuit/nanosuit.obj","",1,0}},
	{"mir", {"model/Male03/Male02.FBX","",0.05,-5}},
	{"spaceship", {"model/Spaceship/Spaceship.fbx","",0.01,0}},
	{"rock", {"model/rock/rock.obj","",1,0}},
	{"floor", {"model/floor/floor.obj","",0.3,0}},
	{"planet", {"model/planet/planet.obj","",0.1,0}},
};
model::model()
{
	mScale = Eigen::Vector3f::Ones();
	mPos = Eigen::Vector3f::Zero();
}
model::model(const std::string& name) : model()
{
	Init(name);
}
void model::Init(const std::string& name)
{
	mName = name;
	auto iter = CResPathMap.find(name);
	if (iter != CResPathMap.end()) {
		mPath = iter->second.mPath;
		float s = iter->second.mScale;
		mScale = Eigen::Vector3f(s, s, s);
		mPos = Eigen::Vector3f(0, iter->second.mPosY, 0);
	}
}
mir::TransformPtr model::Init(const std::string& name, mir::AssimpModelPtr aiModel)
{
	Init(name);
	aiModel->LoadModel(Path(), Rd());
	mir::TransformPtr transform = aiModel->GetTransform();
	transform->SetScale(Scale());
	transform->SetPosition(Pos());
	return transform;
}

namespace model_nanosuit {
std::string Path() { return "model/nanosuit/nanosuit.obj"; }
std::string Rd() { return R"({"ext":"png","dir":"model/nanosuit/"})"; }
constexpr float scale = 1;
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
constexpr float scale = 0.01;
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

namespace model_floor {
std::string Path() { return "model/floor/floor.obj"; }
std::string Rd() { return R"({"dir":"model/floor/"})"; }
constexpr float scale = 0.3;
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

namespace model_planet {
std::string Path() { return "model/planet/planet.obj"; }
std::string Rd() { return R"({"dir":"model/planet/"})"; }
constexpr float scale = 0.1;
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
}
}
