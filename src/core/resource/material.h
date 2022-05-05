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
#include "core/resource/material_property.h"
#include "core/resource/material_parameter.h"

namespace mir {
namespace res {

class Pass : public ImplementResource<IResource>
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddSampler(ISamplerStatePtr sampler);
	bool Validate() const;

	IInputLayoutPtr GetInputLayout() const { return mInputLayout; }
	IProgramPtr GetProgram() const { return mProgram; }
	std::vector<ISamplerStatePtr> GetSamplers() const { return mSamplers; }

	PrimitiveTopology GetTopoLogy() const { return mProperty->TopoLogy; }
	const std::string& GetLightMode() const { return mProperty->LightMode; }
	const PassProperty::GrabInput& GetGrabIn() const { return mProperty->GrabIn; }
	const PassProperty::GrabOutput& GetGrabOut() const { return mProperty->GrabOut; }
	const PassProperty::ParameterRelation& GetRelateToParam() const { return mProperty->Relate2Parameter; }
private:
	IInputLayoutPtr mInputLayout;
	IProgramPtr mProgram;
	std::vector<ISamplerStatePtr> mSamplers;
	PassPropertyPtr mProperty;
};

class Technique : public tpl::Vector<PassPtr, ImplementResource<IResource>>
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddPass(PassPtr pass) { Add(pass); }
	bool Validate() const;

	std::vector<PassPtr> GetPassesByLightMode(const std::string& lightMode);
};

class MIR_CORE_API Shader : public tpl::Vector<TechniquePtr, ImplementResource<IResource>>
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddTechnique(TechniquePtr technique) { Add(technique); }
	bool Validate() const;

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
	void SetProperty(const std::string& propertyName, const Data& data) {
		mGpuParameters->SetProperty(propertyName, data);
	}
	TemplateT void SetProperty(const std::string& propertyName, const T& value) {
		if (HasProperty(propertyName))
			mGpuParameters->GetProperty<T>(propertyName) = value;
	}
	TemplateT void SetPropertyAt(const std::string& propertyName, size_t pos, float value) {
		T varProp = this->GetProperty<T>(propertyName);
		varProp[pos] = value;
		this->SetProperty(propertyName, varProp);
	}
	void SetPropertyVec4At(const std::string& propertyName, size_t pos, float value) {
		SetPropertyAt<Eigen::Vector4f>(propertyName, pos, value);
	}
	std::vector<IContantBufferPtr> GetConstBuffers() const;
	void FlushGpuParameters(RenderSystem& renderSys);
	void WriteToCb(RenderSystem& renderSys, const std::string& cbName, Data data);

	const MaterialPtr& GetMaterial() const { return mMaterial;}
	MaterialPtr& operator->() { return mMaterial; }
	const MaterialPtr& operator->() const { return mMaterial; };
	operator bool() const { return mMaterial != nullptr; }
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
	CoTask<bool> Build(Launch launchMode, ResourceManager& resMng);
public:
	MaterialInstance CreateInstance(Launch launchMode, ResourceManager& resMng) const;
	MaterialLoadParam GetLoadParam() const { return mShaderVariantParam; }
	const ShaderPtr& GetShader() const { return mShaderVariant; }
	TextureVector& GetTextures() { return mTextures; }
	const TextureVector& GetTextures() const { return mTextures; }
	bool IsOutOfDate() const { return mProperty->DependSrc.CheckOutOfDate(); }
private:
	MaterialLoadParamBuilder mShaderVariantParam;
	ShaderPtr mShaderVariant;
	MaterialPropertyPtr mProperty;
	TextureVector mTextures;
	GpuParametersPtr mGpuParametersByShareType[kCbShareMax];
};

}
}