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

class MIR_CORE_API MaterialFactory : boost::noncopyable
{
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	MaterialFactory(ResourceManager& resMng, const std::string& shaderDir);
	~MaterialFactory();

	CoTask<bool> CreateShader(ShaderPtr& shader, Launch lchMode, MaterialLoadParam loadParam) ThreadSafe;
	CoTask<bool> CreateMaterial(MaterialPtr& material, Launch lchMode, MaterialLoadParam loadParam) ThreadSafe;
	CoTask<bool> CreateMaterial(res::MaterialInstance& mtlInst, Launch lchMode, MaterialLoadParam loadParam) ThreadSafe;
	DECLARE_COTASK_FUNCTIONS(ShaderPtr, CreateShader, ThreadSafe);
	DECLARE_COTASK_FUNCTIONS(MaterialPtr, CreateMaterial, ThreadSafe);

	bool PurgeOutOfDates() ThreadSafe;
	void PurgeAll() ThreadSafe;

	ShaderPtr CloneShader(Launch launch, const Shader& material) ThreadSafe;
	TechniquePtr CloneTechnique(Launch launch, const Technique& technique) ThreadSafe;
	PassPtr ClonePass(Launch launch, const Pass& pass) ThreadSafe;
public:
	const GpuParametersPtr& GetFrameGpuParameters() const ThreadSafe;
private:
	CoTask<bool> DoCreateShaderByShaderNode(ShaderPtr shader, Launch launchMode, mat_asset::ShaderNode shaderNode) ThreadSafe;
	CoTask<bool> DoCreateMaterialByMtlNode(MaterialPtr material, Launch launchMode, mat_asset::MaterialNode materialNode) ThreadSafe;
	CoTask<bool> DoCreateMaterial(MaterialPtr& material, Launch launch, MaterialLoadParam loadParam) ThreadSafe;
	GpuParameters::Element DoCreateGpuParameterElement(Launch launchMode, const UniformParameters& parameters) ThreadSafe;
private:
	ResourceManager& mResMng;
	std::shared_ptr<mat_asset::MaterialAssetManager> mMatAssetMng;
	std::map<std::string, GpuParameters::Element> mParametersByUniformName;
	GpuParametersPtr mFrameGpuParameters;
	tpl::AtomicMap<MaterialLoadParam, res::MaterialPtr> mMaterialCache;
};

#define kTextureUserSlotFirst 0
#define kTextureUserSlotLast 5
#define kTextureUserSlotCount 6

}
}