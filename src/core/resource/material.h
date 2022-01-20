#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/declare_macros.h"
#include "core/base/data.h"
#include "core/base/base_type.h"
#include "core/base/tpl/vector.h"
#include "core/base/material_load_param.h"
#include "core/rendersys/texture.h"
#include "core/resource/resource.h"
#include "core/resource/material_parameter.h"

namespace mir {
namespace res {

#define USE_CBUFFER_ENTRY 1
class Pass : public ImplementResource<IResource>
{
	friend class MaterialFactory;
	struct CBufferEntry {
		MIR_MAKE_ALIGNED_OPERATOR_NEW;
		bool IsValid() const { return Buffer != nullptr; }
	public:
		IContantBufferPtr Buffer;
		std::string Name;
		bool GetShareMode;
	};
	struct CBufferEntryVector : public tpl::Vector<CBufferEntry> {};
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddConstBuffer(IContantBufferPtr buffer, const std::string& name, bool isUnique, int slot);
	void AddSampler(ISamplerStatePtr sampler);
	void UpdateConstBufferByName(RenderSystem& renderSys, const std::string& name, const Data& data);

	void GetLoadDependencies(std::vector<IResourcePtr>& depends) override;
	std::vector<IContantBufferPtr> GetConstBuffers() const;
	IContantBufferPtr GetConstBufferByIdx(size_t idx);
	IContantBufferPtr GetConstBufferByName(const std::string& name);
public:
	std::string mLightMode, mName;
	PrimitiveTopology mTopoLogy;
	IInputLayoutPtr mInputLayout;
	IProgramPtr mProgram;
	std::vector<ISamplerStatePtr> mSamplers;
#if USE_CBUFFER_ENTRY
	CBufferEntryVector mConstantBuffers;//to remove
#endif
	IFrameBufferPtr mFrameBuffer;//to remove
};

class Technique : public tpl::Vector<PassPtr, ImplementResource<IResource>>
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddPass(PassPtr pass) { Add(pass); }

	TemplateArgs void UpdateConstBufferByName(T &&...args) {
		for (auto& pass : mElements)
			pass->UpdateConstBufferByName(std::forward<T>(args)...);
	}

	void GetLoadDependencies(std::vector<IResourcePtr>& depends) override;
	std::vector<PassPtr> GetPassesByLightMode(const std::string& lightMode);
};

class MIR_CORE_API Shader : public tpl::Vector<TechniquePtr, ImplementResource<IResource>>
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddTechnique(TechniquePtr technique) { Add(technique); }

	void GetLoadDependencies(std::vector<IResourcePtr>& depends) override;
	TechniquePtr CurTech() const { return mElements[mCurTechIdx]; }
private:
	int mCurTechIdx = 0;
};

class MIR_CORE_API MaterialInstance {
public:
	MaterialInstance();
	MaterialInstance(const MaterialPtr& material, const GpuParametersPtr& gpuParamters);
public:
	MaterialPtr mMaterial;
	GpuParametersPtr mGpuParameters;
};

class MIR_CORE_API Material : public ImplementResource<IResource>, std::enable_shared_from_this<Material>
{
	friend class MaterialFactory;
public:
	Material();
	void EnableKeyword(const std::string& macroName, int value = TRUE);
	void Build(Launch launchMode, ResourceManager& resMng);
public:
	const ShaderPtr& GetShader() const;
	MaterialInstance CreateInstance(Launch launchMode, ResourceManager& resMng);
private:
	ShaderLoadParamBuilder mShaderVariantParam;
	ShaderPtr mShaderVariant;
	TextureVector mTextures;
	GpuParametersPtr mGpuParametersByShareType[kCbShareMax];
};

}
}