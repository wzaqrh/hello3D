#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/tpl/atomic_map.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/material_load_param.h"
#include "core/resource/material.h"
#include "core/resource/material_parameter.h"

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
	CoTask<ShaderPtr> CreateShader(Launch launch, ResourceManager& resMng, const MaterialLoadParam& loadParam, ShaderPtr shader = nullptr) ThreadSafe;
	CoTask<MaterialPtr> CreateMaterial(Launch launch, ResourceManager& resMng, const MaterialLoadParam& loadParam, MaterialPtr material = nullptr) ThreadSafe;

	ShaderPtr CloneShader(Launch launch, ResourceManager& resMng, const Shader& material);
	TechniquePtr CloneTechnique(Launch launch, ResourceManager& resMng, const Technique& technique);
	PassPtr ClonePass(Launch launch, ResourceManager& resMng, const Pass& pass);
public:
	const GpuParametersPtr& GetFrameGpuParameters() const { return mFrameGpuParameters; }
private:
	CoTask<ShaderPtr> DoCreateShader(Launch launchMode, ResourceManager& resMng, const mat_asset::ShaderNode& shaderNode, ShaderPtr shader) ThreadSafe;
	CoTask<MaterialPtr> DoCreateMaterial(Launch launchMode, ResourceManager& resMng, const mat_asset::MaterialNode& materialNode, MaterialPtr material) ThreadSafe;
	GpuParameters::Element AddToParametersCache(Launch launchMode, ResourceManager& resMng, const UniformParameters& parameters) ThreadSafe;
private:
	std::shared_ptr<mat_asset::MaterialAssetManager> mMatAssetMng;
	tpl::AtomicMap<std::string, GpuParameters::Element> mParametersCache;
	GpuParametersPtr mFrameGpuParameters;
};

}
}