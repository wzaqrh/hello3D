#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/launch.h"
#include "core/resource/material.h"

namespace mir {

struct MaterialAsset;
struct MaterialFactory : boost::noncopyable
{
	std::shared_ptr<class MaterialAssetManager> mMatAssetMng;
public:
	MaterialFactory();
	MaterialPtr CreateMaterial(Launch launchMode, ResourceManager& resourceMng, const std::string& matName);
	
	MaterialPtr CloneMaterial(Launch launchMode, ResourceManager& resourceMng, const Material& material);
	TechniquePtr CloneTechnique(Launch launchMode, ResourceManager& resourceMng, const Technique& technique);
	PassPtr ClonePass(Launch launchMode, ResourceManager& resourceMng, const Pass& pass);
private:
	MaterialPtr CreateMaterialByMaterialAsset(Launch launchMode, ResourceManager& resourceMng, const MaterialAsset& matAsset);
};

}