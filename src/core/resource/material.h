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

#if !USE_MATERIAL_INSTANCE
#define USE_CBUFFER_ENTRY 1
#endif
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
	void AddSampler(ISamplerStatePtr sampler);
#if USE_CBUFFER_ENTRY
	void AddConstBuffer(IContantBufferPtr buffer, const std::string& name, bool isUnique, int slot);
	void UpdateConstBufferByName(RenderSystem& renderSys, const std::string& name, const Data& data);

	std::vector<IContantBufferPtr> GetConstBuffers() const;
	IContantBufferPtr GetConstBufferByIdx(size_t idx);
	IContantBufferPtr GetConstBufferByName(const std::string& name);
#endif
	void GetLoadDependencies(std::vector<IResourcePtr>& depends) override;
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

#if USE_CBUFFER_ENTRY
	TemplateArgs void UpdateConstBufferByName(T &&...args) {
		for (auto& pass : mElements)
			pass->UpdateConstBufferByName(std::forward<T>(args)...);
	}
#endif

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

	//operate per-instance¡¢per-material¡¢per-frame properties
	TemplateArgs bool HasProperty(const std::string& propertyName) { return mGpuParameters->HasProperty(propertyName); }
	TemplateT T& GetProperty(const std::string& propertyName) { return mGpuParameters->GetProperty<T>(propertyName); }
	TemplateT const T& GetProperty(const std::string& propertyName) const { return mGpuParameters->GetProperty<T>(propertyName); }
	TemplateT void SetProperty(const std::string& propertyName, const T& value) {
		if (HasProperty(propertyName))
			mGpuParameters->GetProperty<T>(propertyName) = value;
	}
	std::vector<IContantBufferPtr> GetConstBuffers() const;
	void FlushGpuParameters(RenderSystem& renderSys) const;
	void WriteToCb(RenderSystem& renderSys, const std::string& cbName, Data data) const;

	const MaterialPtr& GetMaterial() const { return mMaterial;}
	MaterialPtr& operator->() { return mMaterial; }
	const MaterialPtr& operator->() const { return mMaterial; };
public:
	MaterialPtr mMaterial;
	GpuParametersPtr mGpuParameters;
};

class MIR_CORE_API Material : public std::enable_shared_from_this<Material>, public ImplementResource<IResource>
{
	friend class MaterialFactory;
public:
	Material();
	void EnableKeyword(const std::string& macroName, int value = TRUE);
	void Build(Launch launchMode, ResourceManager& resMng);
public:
	MaterialInstance CreateInstance(Launch launchMode, ResourceManager& resMng) const;
	const ShaderPtr& GetShader() const;
	TextureVector& GetTextures() { return mTextures; }
	const TextureVector& GetTextures() const { return mTextures; }
	void GetLoadDependencies(std::vector<IResourcePtr>& depends) override;
private:
	ShaderLoadParamBuilder mShaderVariantParam;
	ShaderPtr mShaderVariant;
	TextureVector mTextures;
	GpuParametersPtr mGpuParametersByShareType[kCbShareMax];
};

}
}