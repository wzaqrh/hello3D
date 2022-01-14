#pragma once
#include <boost/noncopyable.hpp>
#include "core/mir_export.h"
#include "core/predeclare.h"
#include "core/base/declare_macros.h"
#include "core/base/base_type.h"
#include "core/rendersys/texture.h"
#include "core/resource/resource.h"

namespace mir {

struct CBufferEntry 
{
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	static CBufferEntry MakeEmpty() {
		return CBufferEntry{nullptr, "", false};
	}
	static CBufferEntry Make(IContantBufferPtr buffer, const std::string& name, bool isUnique) {
		return CBufferEntry{buffer, name, isUnique};
	}
	bool IsValid() const { return Buffer != nullptr; }
public:
	IContantBufferPtr Buffer;
	std::string Name;
	bool IsUnique;
};
#define MAKE_CBNAME(V) #V

class Pass : public ImplementResource<IResource> 
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddConstBuffer(const CBufferEntry& cbuffer, int slot);
	void AddSampler(ISamplerStatePtr sampler);
	void UpdateConstBufferByName(RenderSystem& renderSys, const std::string& name, const Data& data);

	std::vector<IContantBufferPtr> GetConstBuffers() const;
	IContantBufferPtr GetConstBufferByIdx(size_t idx);
	IContantBufferPtr GetConstBufferByName(const std::string& name);
public:
	std::string mLightMode, mName;
	PrimitiveTopology mTopoLogy;
	IInputLayoutPtr mInputLayout;
	std::vector<ISamplerStatePtr> mSamplers;
	std::vector<CBufferEntry> mConstantBuffers;
	IProgramPtr mProgram;
	IFrameBufferPtr mFrameBuffer;
};

class Technique : public ImplementResource<IResource>
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddPass(PassPtr pass) {
		mPasses.push_back(pass);
	}
	TemplateArgs void UpdateConstBufferByName(T &&...args) {
		for (auto& pass : mPasses)
			pass->UpdateConstBufferByName(std::forward<T>(args)...);
	}

	PassPtr GetPassByLightMode(const std::string& lightMode);
	std::vector<PassPtr> GetPassesByLightMode(const std::string& lightMode);
public:
	std::vector<PassPtr> mPasses;
};

class MIR_CORE_API Material : public ImplementResource<IResource> 
{
	friend class MaterialFactory;
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	void AddTechnique(TechniquePtr technique) {
		mTechniques.push_back(technique);
	}
	TechniquePtr SetCurTechByIdx(int idx);

	TechniquePtr CurTech() const { return mTechniques[mCurTechIdx]; }
private:
	std::vector<TechniquePtr> mTechniques;
	int mCurTechIdx = 0;
};

}