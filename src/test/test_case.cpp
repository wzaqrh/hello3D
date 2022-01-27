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

#define MakeKhronosGltfEnvPath(Name) "gltf/env/" Name "/" Name ".dds"
std::string Sky(int index) { 
	std::string skyArr[] = {
		"uffizi_cross.dds",
		"footprint_court.dds",
		//MakeKhronosGltfEnvPath("pisa"),
		MakeKhronosGltfEnvPath("footprint_court")
	};
	return "model/" + skyArr[index];
}

#define MakeKhronosGltfEnvPath1(Name, file) "gltf/env/" Name "/" file
namespace sky {
namespace footprint_court {
std::string Diffuse() {
	return "model/" MakeKhronosGltfEnvPath1("footprint_court", "diffuse_env.dds");
}
std::string Specular() {
	return "model/" MakeKhronosGltfEnvPath1("footprint_court", "specular_env.dds");
}
}
}

namespace cube {
namespace far_plane {
cppcoro::shared_task<mir::renderable::CubePtr> Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname) {
	constexpr int SizeBig = 8192;
	return co_await rendFac->CreateCube(
		Eigen::Vector3f(0, 0, test1::cam::Far()),
		Eigen::Vector3f(SizeBig, SizeBig, 1),
		0xffff6347,
		matname
	);
}
}
namespace near_plane {
cppcoro::shared_task<mir::renderable::CubePtr> Create(mir::RenderableFactoryPtr rendFac, Eigen::Vector3f winCenter, const mir::MaterialLoadParam& matname) {
	constexpr int SizeSmall = 4;
	return co_await rendFac->CreateCube(
		Eigen::Vector3f(0, 0, test1::cam::Near()),
		Eigen::Vector3f(SizeSmall, SizeSmall, 1),
		0xffff4763,
		matname
	);
}
}
namespace floor {
cppcoro::shared_task<mir::renderable::CubePtr> Create(mir::RenderableFactoryPtr rendFac, float y, const mir::MaterialLoadParam& matname) {
	constexpr int Inf = 65536;
	return co_await rendFac->CreateCube(
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
void SetPos(mir::renderable::SpritePtr sprite, Eigen::Vector3f pos, Eigen::Vector3f size, Eigen::Vector3f anchor) {
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
#define MakeKhronosGltfResPath(Name) "model/gltf/2.0/" Name "/glTF/" Name ".gltf"
#define MakeResPath1(Name1, Name2, Postfix) "model/" Name1 "/" Name2 "." Postfix
#define MakeResPath(Name, Postfix) MakeResPath1(Name, Name, Postfix)
std::map<std::string, ModelInfo> CResPathMap = {
	{"nanosuit", {MakeResPath("nanosuit", "obj"),"",1,0}},
	{"mir", {MakeResPath1("Male03", "Male02", "fbx"),"",0.05,-5}},
	{"spaceship", {MakeResPath("Spaceship", "fbx"),"",0.01,0}},
	{"rock", {MakeResPath("rock", "obj"),"",1,0}},
	{"floor", {MakeResPath("floor", "obj"),"",0.3,0}},
	{"planet", {MakeResPath("planet", "obj"),"",0.1,0}},
	{"toycar", {MakeKhronosGltfResPath("ToyCar"),"",1,0}},
	{"box-space", {MakeKhronosGltfResPath("Box With Spaces"),"",1,0}},
	{"damaged-helmet", {MakeKhronosGltfResPath("DamagedHelmet"),"",1,0}},
	{"BoomBox", {MakeKhronosGltfResPath("BoomBox"),"",100,0}},
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
	else {
		mPath = "model/gltf/2.0/" + name + "/glTF/" + name + ".gltf";
		float s = 1;
		mScale = Eigen::Vector3f(s, s, s);
		mPos = Eigen::Vector3f(0, 0, 0);
	}
}
cppcoro::shared_task<mir::TransformPtr> model::Init(const std::string& name, mir::renderable::AssimpModelPtr aiModel)
{
	Init(name);
	co_await aiModel->LoadModel(Path(), Rd());
	mir::TransformPtr transform = aiModel->GetTransform();
	transform->SetScale(Scale());
	transform->SetPosition(Pos());
	return transform;
}
}
}
