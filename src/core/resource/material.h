#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/cppcoro.h"
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
	void GetLoadDependencies(std::vector<IResourcePtr>& depends) override;
public:
	std::string mLightMode, mName;
	PrimitiveTopology mTopoLogy;
	IInputLayoutPtr mInputLayout;
	IProgramPtr mProgram;
	std::vector<ISamplerStatePtr> mSamplers;
	IFrameBufferPtr mFrameBuffer;//to remove
};

class Technique : public tpl::Vector<PassPtr, ImplementResource<IResource>>
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddPass(PassPtr pass) { Add(pass); }

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
	cppcoro::shared_task<bool> Build(Launch launchMode, ResourceManager& resMng);
public:
	MaterialInstance CreateInstance(Launch launchMode, ResourceManager& resMng) const;
	const ShaderPtr& GetShader() const;
	TextureVector& GetTextures() { return mTextures; }
	const TextureVector& GetTextures() const { return mTextures; }
	void GetLoadDependencies(std::vector<IResourcePtr>& depends) override;
private:
	MaterialLoadParamBuilder mShaderVariantParam;
	ShaderPtr mShaderVariant;
	TextureVector mTextures;
	GpuParametersPtr mGpuParametersByShareType[kCbShareMax];
};

}
}