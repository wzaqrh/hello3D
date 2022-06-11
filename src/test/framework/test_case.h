#pragma once
#include "test/framework/app.h"
#include "test/framework/create_app.h"
#include "core/mir.h"
#include "core/resource/material_name.h"
#include "core/renderable/assimp_model.h"
#include "core/scene/light.h"
#include "core/scene/camera.h"
#include "core/scene/transform.h"
#include "core/scene/scene_manager.h"

using namespace mir;
using namespace mir::rend;
using namespace mir::scene;

namespace test1 {

namespace res {

void SetMediaDirectory(const std::string& dir);

std::vector<std::string> Sky(std::string name = "footprint_court");

std::string Image(std::string name);
void SetPos(mir::rend::SpritePtr sp, Eigen::Vector3f pos, Eigen::Vector3f size, Eigen::Vector3f anchor = mir::math::vec::anchor::LeftBottom());

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
