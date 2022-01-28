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
	CoTask<bool> CreateShader(Launch launch, ShaderPtr& shader, ResourceManager& resMng, const MaterialLoadParam& loadParam) ThreadSafe;
	CoTask<bool> CreateMaterial(Launch launch, MaterialPtr& material, ResourceManager& resMng, const MaterialLoadParam& loadParam) ThreadSafe;

	ShaderPtr CloneShader(Launch launch, ResourceManager& resMng, const Shader& material);
	TechniquePtr CloneTechnique(Launch launch, ResourceManager& resMng, const Technique& technique);
	PassPtr ClonePass(Launch launch, ResourceManager& resMng, const Pass& pass);
public:
	const GpuParametersPtr& GetFrameGpuParameters() const { return mFrameGpuParameters; }
private:
	CoTask<bool> DoCreateShader(Launch launchMode, ShaderPtr shader, ResourceManager& resMng, const mat_asset::ShaderNode& shaderNode) ThreadSafe;
	CoTask<bool> DoCreateMaterial(Launch launchMode, MaterialPtr material, ResourceManager& resMng, const mat_asset::MaterialNode& materialNode) ThreadSafe;
	GpuParameters::Element AddToParametersCache(Launch launchMode, ResourceManager& resMng, const UniformParameters& parameters) ThreadSafe;
private:
	std::shared_ptr<mat_asset::MaterialAssetManager> mMatAssetMng;
	tpl::AtomicMap<std::string, GpuParameters::Element> mParametersCache;
	GpuParametersPtr mFrameGpuParameters;
};

}
}