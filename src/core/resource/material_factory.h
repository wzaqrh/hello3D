#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/resource/material.h"

namespace mir {
namespace res {

namespace mat_asset {
struct ShaderNode;
struct MaterialNode;
class MaterialAssetManager;
}
class MaterialFactory : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	MaterialFactory();
	ShaderPtr CreateShader(Launch launch, ResourceManager& resMng, const ShaderLoadParam& loadParam, ShaderPtr shader = nullptr);
	MaterialPtr CreateMaterial(Launch launch, ResourceManager& resMng, const std::string& loadPath, MaterialPtr material = nullptr);

	ShaderPtr CloneShader(Launch launch, ResourceManager& resMng, const Shader& material);
	TechniquePtr CloneTechnique(Launch launch, ResourceManager& resMng, const Technique& technique);
	PassPtr ClonePass(Launch launch, ResourceManager& resMng, const Pass& pass);
private:
	ShaderPtr DoCreateShader(Launch launchMode, ResourceManager& resMng, const mat_asset::ShaderNode& shaderNode, ShaderPtr shader);
	MaterialPtr DoCreateMaterial(Launch launchMode, ResourceManager& resMng, const mat_asset::MaterialNode& materialNode, MaterialPtr material);
private:
	std::shared_ptr<mat_asset::MaterialAssetManager> mMatAssetMng;
};

}
}