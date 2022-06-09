#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/tpl/vector.h"
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/cppcoro.h"
#include "core/base/declare_macros.h"
#include "core/base/data.h"
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

	int GetLightMode() const { return mProperty->LightMode; }
	const PassProperty::GrabInput& GetGrabIn() const { return mProperty->GrabIn; }
	const PassProperty::GrabOutput& GetGrabOut() const { return mProperty->GrabOut; }
	const PassProperty::ParameterRelation& GetRelateToParam() const { return mProperty->Relate2Parameter; }

	PrimitiveTopology GetTopoLogy() const { return mProperty->TopoLogy; }
	const std::optional<BlendState>& GetBlend() const { return mProperty->Blend; }
	const std::optional<DepthState>& GetDepth() const { return mProperty->Depth; }
	const std::optional<FillMode>& GetFill() const { return mProperty->Fill; }
	const std::optional<CullMode>& GetCull() const { return mProperty->Cull; }
	const std::optional<DepthBias>& GetDepthBias() const { return mProperty->DepthBias; }
	const std::optional<ScissorState>& GetScissor() const { return mProperty->Scissor; }
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

	std::vector<PassPtr> GetPassesByLightMode(int lightMode);
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

class MIR_CORE_API MaterialInstance 
{
	friend struct KeywordOperator;
public:
	MaterialInstance() {}
	MaterialInstance(const MaterialPtr& material, const TextureVector& textures, const GpuParametersPtr& gpuParamters, const MaterialLoadParam& loadParam);
	MaterialInstance Clone(Launch launchMode, ResourceManager& resMng) const;
	MaterialInstance ShallowClone() const;
	CoTask<bool> Reload(Launch launchMode, ResourceManager& resMng);

	/********** about keywords **********/
	struct KeywordOperator {
		KeywordOperator(MaterialInstance& mtl, Launch lchMode, ResourceManager& resMng) :mMtl(mtl), mLchMode(lchMode), mResMng(resMng), mOwn(true), mFlush(false) {}
		KeywordOperator(KeywordOperator&& other) :mMtl(other.mMtl), mLchMode(other.mLchMode), mResMng(other.mResMng), mOwn(true), mFlush(other.mFlush) { other.mOwn = false; }
		~KeywordOperator() { if (mOwn && mFlush) mMtl.CommitKeywords(mLchMode, mResMng); }
		KeywordOperator(const KeywordOperator& other) = delete;
		KeywordOperator& operator=(KeywordOperator&& other) = delete;
		void operator()(const std::string& macroName, int value) { mMtl.UpdateKeyword(macroName, value); mFlush = true; }
	private:
		Launch mLchMode;
		ResourceManager& mResMng;
		MaterialInstance& mMtl;
		bool mOwn, mFlush;
	};
	KeywordOperator OperateKeywords(Launch lchMode, ResourceManager& resMng) { return KeywordOperator(*this, lchMode, resMng); }

	/********** about material **********/
	const MaterialPtr& GetMaterial() const { return mSelf ? mSelf->Material : nullptr; }
	const MaterialPtr& operator->() const { return GetMaterial(); };
	operator bool() const { return mSelf && mSelf->Material != nullptr; }

	/********** about textures **********/
	TextureVector& GetTextures() { return mSelf->Textures; }
	const TextureVector& GetTextures() const { return mSelf->Textures; }

	/********** about gpu parameter **********/
	//operate per-instance¡¢per-material¡¢per-frame parameters
	void SetProperty(const std::string& propertyName, const Data& data) {
		mSelf->GpuParameters->SetProperty(propertyName, data);
	}
	TemplateT void SetProperty(const std::string& propertyName, const T& value) {
		if (HasProperty(propertyName))
			mSelf->GpuParameters->GetProperty<T>(propertyName) = value;
	}
	TemplateT void SetPropertyAt(const std::string& propertyName, size_t pos, float value) {
		T varProp = this->GetProperty<T>(propertyName);
		varProp[pos] = value;
		this->SetProperty(propertyName, varProp);
	}
	void SetPropertyVec4At(const std::string& propertyName, size_t pos, float value) {
		SetPropertyAt<Eigen::Vector4f>(propertyName, pos, value);
	}
	
	TemplateArgs bool HasProperty(const std::string& propertyName) { return mSelf->GpuParameters->HasProperty(propertyName); }
	TemplateT T& GetProperty(const std::string& propertyName) { return mSelf->GpuParameters->GetProperty<T>(propertyName); }
	TemplateT const T& GetProperty(const std::string& propertyName) const { return mSelf->GpuParameters->GetProperty<T>(propertyName); }
	
	//flush parameters
	void FlushGpuParameters(RenderSystem& renderSys);
	void WriteToCb(RenderSystem& renderSys, const std::string& cbName, Data data);
	std::vector<IContantBufferPtr> GetConstBuffers() const;
private:
	void UpdateKeyword(const std::string& macroName, int value = TRUE);
	CoTask<bool> CommitKeywords(Launch launchMode, ResourceManager& resMng);
private:
	struct SharedBlock {
		SharedBlock(const MaterialPtr& material, const TextureVector& textures, const GpuParametersPtr& gpuParamters, const MaterialLoadParam& loadParam)
			:Material(material), Textures(textures), GpuParameters(gpuParamters), LoadParam(loadParam) {}
		MaterialPtr Material;
		TextureVector Textures;
		GpuParametersPtr GpuParameters;
		MaterialLoadParamBuilder LoadParam;
	};
	std::shared_ptr<SharedBlock> mSelf;
};

class MIR_CORE_API Material : public std::enable_shared_from_this<Material>, public ImplementResource<IResource>
{
	friend class MaterialFactory;
public:
	Material() {}
	MaterialInstance CreateInstance(Launch launchMode, ResourceManager& resMng) const;
	MaterialPtr Clone(Launch launchMode, ResourceManager& resMng) const;
	bool IsOutOfDate() const { return mProperty->DependSrc.CheckOutOfDate(); }
	const ShaderPtr& GetShader() const { return mShader; }
	const MaterialLoadParam& GetLoadParam() const { return mLoadParam; }
	const MaterialProperty& GetProperty() const { return *mProperty; }
private:
	ShaderPtr mShader;
	TextureVector mTextures;
	GpuParametersPtr mGpuParametersByShareType[kCbShareMax];
	MaterialPropertyPtr mProperty;
	MaterialLoadParam mLoadParam;
};

}
}