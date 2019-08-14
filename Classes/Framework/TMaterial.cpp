#include "TMaterial.h"
#include "TMaterialCB.h"
#include "TInterfaceType.h"
#include "IRenderSystem.h"
#include "TPostProcess.h"
#include "Utility.h"

/********** TContantBufferInfo **********/
TContantBufferInfo::TContantBufferInfo(IContantBufferPtr __buffer, const std::string& __name, bool __isUnique)
	:buffer(__buffer)
	,name(__name)
	,isUnique(__isUnique)
{
}

/********** TPass **********/
TPass::TPass(const std::string& lightMode, const std::string& name)
	:mLightMode(lightMode)
	,mName(name)
{
}

IContantBufferPtr TPass::AddConstBuffer(const TContantBufferInfo& cbuffer)
{
	mConstantBuffers.push_back(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr TPass::AddSampler(ISamplerStatePtr sampler)
{
	mSamplers.push_back(sampler);
	return sampler;
}

IRenderTexturePtr TPass::AddIterTarget(IRenderTexturePtr target)
{
	mIterTargets.push_back(target);
	return target;
}

IContantBufferPtr TPass::GetConstBufferByIdx(size_t idx)
{
	IContantBufferPtr ret = nullptr;
	if (idx < mConstantBuffers.size())
		ret = mConstantBuffers[idx].buffer;
	return ret;
}

IContantBufferPtr TPass::GetConstBufferByName(const std::string& name)
{
	IContantBufferPtr ret = nullptr;
	for (size_t i = 0; i < mConstantBuffers.size(); ++i) {
		if (mConstantBuffers[i].name == name) {
			ret = mConstantBuffers[i].buffer;
			break;
		}
	}
	return ret;
}

void TPass::UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data)
{
	IContantBufferPtr buffer = GetConstBufferByName(name);
	if (buffer)
		pRenderSys->UpdateConstBuffer(buffer, data.data, data.dataSize);
}

std::shared_ptr<TPass> TPass::Clone(IRenderSystem* pRenderSys)
{
	TPassPtr pass = std::make_shared<TPass>(mLightMode, mName);
	pass->mInputLayout = mInputLayout;
	pass->mTopoLogy = mTopoLogy;
	pass->mProgram = mProgram;
	
	for (auto& sampler : mSamplers)
		pass->AddSampler(sampler);

	for (size_t i = 0; i < mConstantBuffers.size(); ++i) {
		auto buffer = mConstantBuffers[i];
		if (!buffer.isUnique)
			buffer.buffer = pRenderSys->CloneConstBuffer(buffer.buffer);
		pass->AddConstBuffer(buffer);
	}

	pass->mRenderTarget = mRenderTarget;
	for (auto& target : mIterTargets)
		pass->AddIterTarget(target);
	pass->mTextures = mTextures;

	pass->OnBind = OnBind;
	pass->OnUnbind = OnUnbind;
	return pass;
}

/********** TTechnique **********/
void TTechnique::AddPass(TPassPtr pass)
{
	mPasses.push_back(pass);
}

IContantBufferPtr TTechnique::AddConstBuffer(const TContantBufferInfo& cbuffer)
{
	for (auto& pass : mPasses)
		pass->AddConstBuffer(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr TTechnique::AddSampler(ISamplerStatePtr sampler)
{
	for (auto& pass : mPasses)
		pass->AddSampler(sampler);
	return sampler;
}

TPassPtr TTechnique::GetPassByName(const std::string& passName)
{
	TPassPtr pass;
	for (int i = 0; i < mPasses.size(); ++i) {
		if (mPasses[i]->mLightMode == passName) {
			pass = mPasses[i];
			break;
		}
	}
	return pass;
}

std::vector<TPassPtr> TTechnique::GetPassesByName(const std::string& passName)
{
	std::vector<TPassPtr> passVec;
	for (int i = 0; i < mPasses.size(); ++i) {
		if (mPasses[i]->mLightMode == passName) {
			passVec.push_back(mPasses[i]);
		}
	}
	return std::move(passVec);
}

void TTechnique::UpdateConstBufferByName(IRenderSystem* pRenderSys, const std::string& name, const TData& data)
{
	for (int i = 0; i < mPasses.size(); ++i)
		mPasses[i]->UpdateConstBufferByName(pRenderSys, name, data);
}

std::shared_ptr<TTechnique> TTechnique::Clone(IRenderSystem* pRenderSys)
{
	TTechniquePtr technique = std::make_shared<TTechnique>();
	for (int i = 0; i < mPasses.size(); ++i)
		technique->AddPass(mPasses[i]->Clone(pRenderSys));
	technique->mName = mName;
	return technique;
}

/********** TMaterial **********/
void TMaterial::AddTechnique(TTechniquePtr technique)
{
	mTechniques.push_back(technique);
}

TTechniquePtr TMaterial::CurTech()
{
	return mTechniques[mCurTechIdx];
}

TTechniquePtr TMaterial::SetCurTechByIdx(int idx)
{
	mCurTechIdx = idx;
	return mTechniques[mCurTechIdx];
}

IContantBufferPtr TMaterial::AddConstBuffer(const TContantBufferInfo& cbuffer)
{
	for (auto& tech : mTechniques)
		tech->AddConstBuffer(cbuffer);
	return cbuffer.buffer;
}

ISamplerStatePtr TMaterial::AddSampler(ISamplerStatePtr sampler)
{
	for (auto& tech : mTechniques)
		tech->AddSampler(sampler);
	return sampler;
}

std::shared_ptr<TMaterial> TMaterial::Clone(IRenderSystem* pRenderSys)
{
	TMaterialPtr material = std::make_shared<TMaterial>();
	
	for (auto& depend : mDepends)
		material->AddDependency(depend);
	material->mCurState = mCurState;

	for (int i = 0; i < mTechniques.size(); ++i) {
		material->AddTechnique(mTechniques[i]->Clone(pRenderSys));
	}
	material->mCurTechIdx = mCurTechIdx;
	return material;
}

/********** TMaterialBuilder **********/
TMaterialBuilder::TMaterialBuilder()
{
	mMaterial = std::make_shared<TMaterial>();
	AddTechnique();
	AddPass(E_PASS_FORWARDBASE, "");
}

TMaterialBuilder::TMaterialBuilder(TMaterialPtr material)
{
	mMaterial = material;
	mCurTech = material->CurTech();
	mCurPass = mCurTech->mPasses.empty() ? nullptr : mCurTech->mPasses[mCurTech->mPasses.size()-1];
}

TMaterialBuilder& TMaterialBuilder::AddTechnique()
{
	mCurTech = std::make_shared<TTechnique>();
	mMaterial->AddTechnique(mCurTech);
	return *this;
}

TMaterialBuilder& TMaterialBuilder::AddPass(const std::string& lightMode, const std::string& passName)
{
	mCurPass = std::make_shared<TPass>(lightMode, passName);
	mCurTech->AddPass(mCurPass);
	return *this;
}

TMaterialBuilder& TMaterialBuilder::SetPassName(const std::string& lightMode, const std::string& passName)
{
	mCurPass->mLightMode = lightMode;
	mCurPass->mName = passName;
	return *this;
}

TMaterialBuilder& TMaterialBuilder::SetInputLayout(IInputLayoutPtr inputLayout)
{
	mCurPass->mInputLayout = inputLayout;
	mMaterial->AddDependency(inputLayout);
	return *this;
}

TMaterialBuilder& TMaterialBuilder::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	mCurPass->mTopoLogy = topology;
	return *this;
}

TProgramPtr TMaterialBuilder::SetProgram(TProgramPtr program)
{
	mCurPass->mProgram = program;
	mMaterial->AddDependency(program);
	return program;
}

TMaterialBuilder& TMaterialBuilder::AddSampler(ISamplerStatePtr sampler)
{
	mCurPass->AddSampler(sampler);
	return *this;
}

TMaterialBuilder& TMaterialBuilder::AddConstBuffer(IContantBufferPtr buffer, const std::string& name, bool isUnique)
{
	mCurPass->AddConstBuffer(TContantBufferInfo(buffer, name, isUnique));
	return *this;
}

TMaterialBuilder& TMaterialBuilder::AddConstBufferToTech(IContantBufferPtr buffer, const std::string& name, bool isUnique)
{
	mCurTech->AddConstBuffer(TContantBufferInfo(buffer, name, isUnique));
	return *this;
}

TMaterialBuilder& TMaterialBuilder::SetRenderTarget(IRenderTexturePtr target)
{
	mCurPass->mRenderTarget = target;
	return *this;
}

TMaterialBuilder& TMaterialBuilder::AddIterTarget(IRenderTexturePtr target)
{
	mCurPass->AddIterTarget(target);
	return *this;
}

TMaterialBuilder& TMaterialBuilder::SetTexture(size_t slot, ITexturePtr texture)
{
	mCurPass->mTextures[slot] = texture;
	mMaterial->AddDependency(texture);
	return *this;
}

TMaterialPtr TMaterialBuilder::Build()
{
	mMaterial->CheckAndSetLoaded();
	return mMaterial;
}

/********** TMaterialFactory **********/
TMaterialFactory::TMaterialFactory(IRenderSystem* pRenderSys)
{
	mRenderSys = pRenderSys;
}

TMaterialPtr TMaterialFactory::GetMaterial(std::string name, std::function<void(TMaterialPtr material)> callback /*= nullptr*/, std::string identify /* = ""*/, bool readonly /*= false*/)
{
	TMaterialPtr material;

	if (mMaterials.find(name) == mMaterials.end()) {
		mMaterials.insert(std::make_pair(name, CreateStdMaterial(name)));
	}

	if (!identify.empty()) {
		auto key = name + ":" + identify;
		if (mMaterials.find(key) == mMaterials.end()) {
			material = mMaterials[name]->Clone(mRenderSys);
			if (callback) callback(material);
			mMaterials.insert(std::make_pair(key, material));
		}
		else {
			material = readonly ? mMaterials[key] : mMaterials[key]->Clone(mRenderSys);
		}
	}
	else if (callback != nullptr) {
		material = mMaterials[name]->Clone(mRenderSys);
		callback(material);
	}
	else {
		material = readonly ? mMaterials[name] : mMaterials[name]->Clone(mRenderSys);
	}
	return material;
}

void SetCommonField(TMaterialBuilder& builder, IRenderSystem* pRenderSys)
{
	builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	builder.AddConstBuffer(pRenderSys->CreateConstBuffer(MAKE_CBDESC(cbGlobalParam)));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_ANISOTROPIC));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_POINT));
}

void SetCommonField2(TMaterialBuilder& builder, IRenderSystem* pRenderSys)
{
	builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	builder.AddConstBuffer(pRenderSys->CreateConstBuffer(MAKE_CBDESC(cbGlobalParam)));
}

TMaterialPtr TMaterialFactory::CreateStdMaterial(std::string name)
{
	TIME_PROFILE2(CreateStdMaterial, name);

	TMaterialPtr material;
	TMaterialBuilder builder;
	if (name == E_MAT_SPRITE) {
		SetCommonField(builder, mRenderSys);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 7 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Sprite")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
	}
	else if (name == E_MAT_MODEL) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		SetCommonField(builder, mRenderSys);
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Model")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);
	}
	else if (name == E_MAT_MODEL_PBR) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		//*//pass E_PASS_FORWARDBASE
		builder.SetPassName(E_PASS_FORWARDBASE, "ForwardBase");
		SetCommonField(builder, mRenderSys);
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ModelPbr")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		//*//pass E_PASS_FORWARDADD
		builder.AddPass(E_PASS_FORWARDADD, "ForwardAdd");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ModelPbr"), nullptr, "PSAdd"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);
		cbUnityMaterial cbUnityMat;
		//cbUnityMat._Color = XMFLOAT4(0,0,0,0);
		//cbUnityMat._SpecLightOff = TRUE;
		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbUnityMaterial), &cbUnityMat), MAKE_CBNAME(cbUnityMaterial));
		cbUnityGlobal cbUnityGlb;
		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbUnityGlobal), &cbUnityGlb), MAKE_CBNAME(cbUnityGlobal));

		//*//pass E_PASS_SHADOWCASTER
		builder.AddPass(E_PASS_SHADOWCASTER, "ShadowCaster");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ModelPbr"), "VSShadowCaster", "PSShadowCaster"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);
	}
	else if (name == E_MAT_MODEL_SHADOW) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 6 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 9 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 11 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 15 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, 19 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		//pass E_PASS_FORWARDBASE
		SetCommonField(builder, mRenderSys);
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ShadowMap")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		//pass E_PASS_SHADOWCASTER
		builder.AddPass(E_PASS_SHADOWCASTER, "ShadowCaster");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("ShadowDepth")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		builder.AddConstBufferToTech(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbWeightedSkin)), MAKE_CBNAME(cbWeightedSkin), false);
	}
	else if (name == E_MAT_SKYBOX) {
		SetCommonField2(builder, mRenderSys);
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Skybox")));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		builder.AddSampler(mRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_COMPARISON_ALWAYS));
	}
	else if (name == E_MAT_POSTPROC_BLOOM) {
#define NUM_TONEMAP_TEXTURES  10
#define NUM_BLOOM_TEXTURES    2
		std::vector<IRenderTexturePtr> TexToneMaps(NUM_TONEMAP_TEXTURES);
		int nSampleLen = 1;
		for (size_t i = 0; i < NUM_TONEMAP_TEXTURES; i++) {
			TexToneMaps[i] = mRenderSys->CreateRenderTexture(nSampleLen, nSampleLen, DXGI_FORMAT_R16G16B16A16_UNORM);
			SET_DEBUG_NAME(TexToneMaps[i]->mDepthStencilView, "TexToneMaps"+i);
			nSampleLen *= 2;
		}
		IRenderTexturePtr TexBrightPass = mRenderSys->CreateRenderTexture(mRenderSys->mScreenWidth / 8, mRenderSys->mScreenHeight / 8, DXGI_FORMAT_B8G8R8A8_UNORM);
		SET_DEBUG_NAME(TexBrightPass->mDepthStencilView, "TexBrightPass");
		std::vector<IRenderTexturePtr> TexBlooms(NUM_BLOOM_TEXTURES);
		for (size_t i = 0; i < NUM_BLOOM_TEXTURES; i++) {
			TexBlooms[i] = mRenderSys->CreateRenderTexture(mRenderSys->mScreenWidth / 8, mRenderSys->mScreenHeight / 8, DXGI_FORMAT_R16G16B16A16_UNORM);
			SET_DEBUG_NAME(TexBlooms[i]->mDepthStencilView, "TexBlooms"+i);
		}

		//pass DownScale2x2
		builder.SetPassName(E_PASS_POSTPROCESS, "DownScale2x2");
		SetCommonField(builder, mRenderSys);
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "DownScale2x2"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexToneMaps[NUM_TONEMAP_TEXTURES-1]);
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateDownScale2x2Offsets(mainTex->GetWidth(), mainTex->GetHeight());
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
		};

		//pass DownScale3x3
		builder.AddPass(E_PASS_POSTPROCESS, "DownScale3x3");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "DownScale3x3"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexToneMaps[0]);
		for (int i = 1; i < NUM_TONEMAP_TEXTURES - 1; ++i) {
			builder.AddIterTarget(TexToneMaps[i]);
		}
		builder.SetTexture(0, TexToneMaps[NUM_TONEMAP_TEXTURES - 1]->GetColorTexture());
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateDownScale3x3Offsets(mainTex->GetWidth(), mainTex->GetHeight());
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
		};

		//pass DownScale3x3_BrightPass
		builder.AddPass(E_PASS_POSTPROCESS, "DownScale3x3_BrightPass");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "DownScale3x3_BrightPass"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexBrightPass);
		builder.SetTexture(1, TexToneMaps[0]->GetColorTexture());
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateDownScale3x3Offsets(mainTex->GetWidth(), mainTex->GetHeight());
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
		};

		//pass Bloom
		builder.AddPass(E_PASS_POSTPROCESS, "Bloom");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "BloomPS"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetRenderTarget(TexBlooms[0]);
		for (int i = 1; i < NUM_BLOOM_TEXTURES; ++i) {
			builder.AddIterTarget(TexBlooms[i]);
		}
		builder.SetTexture(1, TexBrightPass->GetColorTexture());
		builder.AddConstBuffer(mRenderSys->CreateConstBuffer(MAKE_CBDESC(cbBloom)), MAKE_CBNAME(cbBloom));
		builder.mCurPass->OnBind = [](TPass* pass, IRenderSystem* pRenderSys, TTextureBySlot& textures) {
			auto mainTex = textures[0];
			cbBloom bloom = cbBloom::CreateBloomOffsets(mainTex->GetWidth(), 3.0f, 1.25f);
			pass->UpdateConstBufferByName(pRenderSys, MAKE_CBNAME(cbBloom), make_data(bloom));
		};

		//pass FinalPass
		builder.AddPass(E_PASS_POSTPROCESS, "FinalPass");
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram(MAKE_MAT_NAME("Bloom"), "VS", "FinalPass"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
		builder.SetTexture(1, TexToneMaps[0]->GetColorTexture());
		builder.SetTexture(2, TexBlooms[0]->GetColorTexture());
	}

	material = builder.Build();
	return material;
}