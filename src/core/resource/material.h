#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/declare_macros.h"
#include "core/base/base_type.h"
#include "core/base/template/container_adapter.h"
#include "core/rendersys/texture.h"
#include "core/resource/resource.h"

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
		bool IsUnique;
	};
	struct CBufferEntryVector : public VectorAdapter<CBufferEntry> {};
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
	std::vector<ISamplerStatePtr> mSamplers;
	CBufferEntryVector mConstantBuffers;
	IProgramPtr mProgram;
	IFrameBufferPtr mFrameBuffer;
};

class Technique : public VectorAdapter<PassPtr, ImplementResource<IResource>>
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

class MIR_CORE_API Shader : public VectorAdapter<TechniquePtr, ImplementResource<IResource>>
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

}
}