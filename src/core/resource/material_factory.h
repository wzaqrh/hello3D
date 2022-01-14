#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/resource/material.h"

namespace mir {

namespace mat_asset {
struct MaterialAsset;
class MaterialAssetManager;
}
struct MaterialFactory : boost::noncopyable
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	MaterialFactory();
	MaterialPtr CreateMaterial(Launch launchMode, ResourceManager& resourceMng, 
		const MaterialLoadParam& matName, MaterialPtr matRes = nullptr);
	
	MaterialPtr CloneMaterial(Launch launchMode, ResourceManager& resourceMng, const Material& material);
	TechniquePtr CloneTechnique(Launch launchMode, ResourceManager& resourceMng, const Technique& technique);
	PassPtr ClonePass(Launch launchMode, ResourceManager& resourceMng, const Pass& pass);
private:
	MaterialPtr CreateMaterialByMaterialAsset(Launch launchMode, ResourceManager& resourceMng, 
		const mat_asset::MaterialAsset& matAsset, MaterialPtr matRes = nullptr);
private:
	std::shared_ptr<mat_asset::MaterialAssetManager> mMatAssetMng;
};

}