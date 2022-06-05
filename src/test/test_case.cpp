#include <boost/math/constants/constants.hpp>
#include "test/test_case.h"
#include "core/scene/transform.h"
#include "core/renderable/assimp_model.h"
#include "core/renderable/sprite.h"

namespace test1 {

namespace res {

std::vector<std::string> Sky(std::string name) {
	std::vector<std::string> vec = {
		"specular.dds",
		"diffuse.dds",
		"lut.png",
		"sheen.dds"
	};
	for (auto& img : vec) {
		img = "model/env/" + name + "/" + img;
	}
	return vec;
}

std::string Image(std::string name) {
	return "model/" + name;
}
void SetPos(mir::rend::SpritePtr sprite, Eigen::Vector3f pos, Eigen::Vector3f size, Eigen::Vector3f anchor) {
	sprite->SetPosition(pos);
	sprite->SetSize(size);
	sprite->SetAnchor(anchor);
}

model::model()
{
	mScale = Eigen::Vector3f::Ones();
	mPos = Eigen::Vector3f::Zero();
}
model::model(const std::string& name) : model()
{
	Init(name);
}
static bool IsFileExits(const std::string& path) {
	FILE* fd = fopen(path.c_str(), "rb");
	if (fd) {
		fclose(fd);
		return true;
	}
	else {
		return false;
	}
}
void model::Init(const std::string& name)
{
	mName = name;

	mPath = "model/gltf/2.0/" + name + "/glTF/" + name + ".gltf";
	FILE* fd = fopen(mPath.c_str(), "rb");
	if (!IsFileExits(mPath)) {
		std::string exts[] = { ".obj", ".gltf", ".fbx" };
		for (auto ext : exts) {
			mPath = "model/" + name + "/" + name + ext;
			if (IsFileExits(mPath))
				break;
		}
	}

	float s = 1;
	mScale = Eigen::Vector3f(s, s, s);
	mPos = Eigen::Vector3f(0, 0, 0);
}
CoTask<mir::TransformPtr> model::Init(const std::string& name, mir::rend::AssimpModelPtr aiModel)
{
	Init(name);
	CoAwait aiModel->LoadModel(Path(), Rd());
	mir::TransformPtr transform = aiModel->GetTransform();
	transform->SetScale(Scale());
	transform->SetPosition(Pos());
	CoReturn transform;
}
}
}
