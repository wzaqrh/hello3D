#include "TMaterial.h"
#include "TRenderSystem.h"

/********** TPass **********/
TPass::TPass(const std::string& passName)
	:mName(passName)
{
}

TContantBufferPtr TPass::AddConstBuffer(TContantBufferPtr buffer)
{
	mConstantBuffers.push_back(buffer);
	mConstBuffers.push_back(buffer->buffer);
	return buffer;
}

ID3D11SamplerState* TPass::AddSampler(ID3D11SamplerState* sampler)
{
	mSamplers.push_back(sampler);
	return sampler;
}

std::shared_ptr<TPass> TPass::Clone()
{
	TPassPtr pass = std::make_shared<TPass>(mName);
	pass->mInputLayout = mInputLayout;
	pass->mTopoLogy = mTopoLogy;
	pass->mProgram = mProgram;
	
	for (auto& sampler : mSamplers)
		pass->AddSampler(sampler);

	for (auto& buffer : mConstantBuffers)
		pass->AddConstBuffer(buffer);
	return pass;
}

/********** TTechnique **********/
void TTechnique::AddPass(TPassPtr pass)
{
	mPasses.push_back(pass);
}

TContantBufferPtr TTechnique::AddConstBuffer(TContantBufferPtr buffer)
{
	for (auto& pass : mPasses)
		pass->AddConstBuffer(buffer);
	return buffer;
}

ID3D11SamplerState* TTechnique::AddSampler(ID3D11SamplerState* sampler)
{
	for (auto& pass : mPasses)
		pass->AddSampler(sampler);
	return sampler;
}

TPassPtr TTechnique::GetPassByName(const std::string& passName)
{
	TPassPtr pass;
	for (int i = 0; i < mPasses.size(); ++i) {
		if (mPasses[i]->mName == passName) {
			pass = mPasses[i];
			break;
		}
	}
	return pass;
}

std::shared_ptr<TTechnique> TTechnique::Clone()
{
	TTechniquePtr technique = std::make_shared<TTechnique>();
	for (int i = 0; i < mPasses.size(); ++i)
		technique->AddPass(mPasses[i]->Clone());
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

TContantBufferPtr TMaterial::AddConstBuffer(TContantBufferPtr buffer)
{
	for (auto& tech : mTechniques)
		tech->AddConstBuffer(buffer);
	return buffer;
}

ID3D11SamplerState* TMaterial::AddSampler(ID3D11SamplerState* sampler)
{
	for (auto& tech : mTechniques)
		tech->AddSampler(sampler);
	return sampler;
}

std::shared_ptr<TMaterial> TMaterial::Clone()
{
	TMaterialPtr material = std::make_shared<TMaterial>();
	for (int i = 0; i < mTechniques.size(); ++i) {
		material->AddTechnique(mTechniques[i]->Clone());
	}
	material->mCurTechIdx = mCurTechIdx;
	return material;
}

/********** TMaterialBuilder **********/
TMaterialBuilder::TMaterialBuilder()
{
	mMaterial = std::make_shared<TMaterial>();
	AddTechnique();
	AddPass(E_PASS_FORWARDBASE);
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

TMaterialBuilder& TMaterialBuilder::AddPass(const std::string& passName)
{
	mCurPass = std::make_shared<TPass>(passName);
	mCurTech->AddPass(mCurPass);
	return *this;
}

TMaterialBuilder& TMaterialBuilder::SetInputLayout(ID3D11InputLayout* inputLayout)
{
	mCurPass->mInputLayout = inputLayout;
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
	return program;
}

TMaterialBuilder& TMaterialBuilder::AddSampler(ID3D11SamplerState* sampler)
{
	mCurPass->AddSampler(sampler);
	return *this;
}

TMaterialBuilder& TMaterialBuilder::AddConstBuffer(TContantBufferPtr buffer)
{
	mCurPass->AddConstBuffer(buffer);
	return *this;
}

TMaterialPtr TMaterialBuilder::Build()
{
	return mMaterial;
}

/********** TMaterialFactory **********/
TMaterialFactory::TMaterialFactory(TRenderSystem* pRenderSys)
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
			material = mMaterials[name]->Clone();
			if (callback) callback(material);
			mMaterials.insert(std::make_pair(key, material));
		}
		else {
			material = readonly ? mMaterials[key] : mMaterials[key]->Clone();
		}
	}
	else if (callback != nullptr) {
		material = mMaterials[name]->Clone();
		callback(material);
	}
	else {
		material = readonly ? mMaterials[name] : mMaterials[name]->Clone();
	}
	return material;
}

void SetCommonField(TMaterialBuilder& builder, TRenderSystem* pRenderSys)
{
	builder.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_ANISOTROPIC));
	builder.AddSampler(pRenderSys->CreateSampler(D3D11_FILTER_MIN_MAG_MIP_POINT));
	builder.AddConstBuffer(pRenderSys->CreateConstBuffer(sizeof(cbGlobalParam)));
}

TMaterialPtr TMaterialFactory::CreateStdMaterial(std::string name)
{
	TMaterialPtr material;
	TMaterialBuilder builder;
	SetCommonField(builder, mRenderSys);
	if (name == E_MAT_STANDARD) {
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 3 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 7 * 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto program = builder.SetProgram(mRenderSys->CreateProgram("shader\\Sprite.fx"));
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
		auto program = builder.SetProgram(mRenderSys->CreateProgram("shader\\Model.fx"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
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
		auto program = builder.SetProgram(mRenderSys->CreateProgram("shader\\ShadowMap.fx"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));

		//pass E_PASS_SHADOWCASTER
		builder.AddPass(E_PASS_SHADOWCASTER);
		SetCommonField(builder, mRenderSys);
		program = builder.SetProgram(mRenderSys->CreateProgram("shader\\ShadowDepth.fx"));
		builder.SetInputLayout(mRenderSys->CreateLayout(program, layout, ARRAYSIZE(layout)));
	}

	material = builder.Build();
	return material;
}
