#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/resource/material.h"

namespace mir {
namespace res {

namespace mat_asset {
struct MaterialAsset;
class MaterialAssetManager;
}
class MaterialFactory : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	MaterialFactory();
	ShaderPtr CreateShader(Launch launchMode, ResourceManager& resourceMng,
		const MaterialLoadParam& matName, ShaderPtr matRes = nullptr);

	ShaderPtr CloneShader(Launch launchMode, ResourceManager& resourceMng, const Shader& material);
	TechniquePtr CloneTechnique(Launch launchMode, ResourceManager& resourceMng, const Technique& technique);
	PassPtr ClonePass(Launch launchMode, ResourceManager& resourceMng, const Pass& pass);
private:
	ShaderPtr CreateMaterialByMaterialAsset(Launch launchMode, ResourceManager& resourceMng,
		const mat_asset::MaterialAsset& matAsset, ShaderPtr matRes = nullptr);
private:
	std::shared_ptr<mat_asset::MaterialAssetManager> mMatAssetMng;
};

}
}