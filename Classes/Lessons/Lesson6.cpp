#include "Lesson6.h"

/********** cbUnityMaterial **********/
cbUnityMaterial::cbUnityMaterial()
{
	_SpecColor = XMFLOAT4(1,1,1,1);
	_Color = XMFLOAT4(1,1,1,1);
	_GlossMapScale = 1;
	_OcclusionStrength = 1;
	_SpecLightOff = 0;
}

/********** cbUnityGlobal **********/
cbUnityGlobal::cbUnityGlobal()
{
	_Unity_IndirectSpecColor = XMFLOAT4(0,0,0,0);
	_AmbientOrLightmapUV = XMFLOAT4(0.01,0.01,0.01,1);
}

//#define PBR_DEBUG
/********** Lesson6 **********/
void Lesson6::OnPostInitDevice()
{
	mRenderSys->SetCamera(45, 150, 1000);

#ifndef PBR_DEBUG
	/*auto light1 = mRenderSys->AddPointLight();
	light1->SetPosition(-50, 50, -100);
	int intensify = 10;
	light1->SetDiffuseColor(intensify, intensify, intensify,1);*/

	/*auto dl1 = mRenderSys->AddDirectLight();
	dl1->SetDirection(-1, 0, 0);

	auto dl2 = mRenderSys->AddDirectLight();
	dl2->SetDirection(1, 0, 0);*/

	//mBackgndColor = XMFLOAT4(0,0,0,0);
	//mRenderSys->SetBlendFunc(TBlendFunc(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA));
	//mRenderSys->SetDepthState(TDepthState(false));

	mRenderSys->AddDirectLight()->SetDirection(1, -1, 1);
	//mRenderSys->AddDirectLight()->SetDirection(0, 0, 1);
#else
	auto light1 = mRenderSys->AddDirectLight();
	light1->SetDirection(-1, 0, 0);

	auto light2 = mRenderSys->AddDirectLight();
	light2->SetDirection(1, 0, 0);

	auto light3 = mRenderSys->AddDirectLight();
	light3->SetDirection(0, 0, 1);
	light3->SetDiffuseColor(3,3,3,1);
#endif

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson6.1.fx", "shader\\Lesson6.1.fx", [&](TMaterialPtr mat) {
		{
			auto buffer = mat->AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(cbUnityMaterial)));
			cbUnityMaterial cb;
			//cb._Color = XMFLOAT4(0,0,0,0);
			mRenderSys->UpdateConstBuffer(buffer, &cb);
		}
		{
			auto buffer = mat->AddConstBuffer(mRenderSys->CreateConstBuffer(sizeof(cbUnityGlobal)));
			cbUnityGlobal cb;
			mRenderSys->UpdateConstBuffer(buffer, &cb);
		}
	});
#ifndef PBR_DEBUG
	gModelPath = "Male03\\"; mModel->LoadModel(MakeModelPath("Male02.FBX")); mScale = 0.3; mPosition = XMFLOAT3(0, -5, 0);

	for (auto& iter : mModel->mMeshes) {
		if (!iter->mTextures.empty()) {
			std::string firstPostfix;
			auto prefix = iter->mTextures[0].path;
			auto pos = prefix.find_last_of("_");
			if (pos != std::string::npos) {
				firstPostfix = prefix.substr(pos + 1, std::string::npos);
				prefix = prefix.substr(0, pos);

				pos = firstPostfix.find_last_of(".");
				if (pos != std::string::npos)
					firstPostfix = firstPostfix.substr(0, pos);
			}

			const char* postfixs[] = {
				"AlbedoTransparency",
				"Normal",
				"Metallic",
				"Smoothness",
				"AO"
			};
			if (postfixs[0] == firstPostfix) {
				iter->mTextures.clear();
				for (int i = iter->mTextures.size(); i < ARRAYSIZE(postfixs); ++i) {
					TTexture texInfo = mRenderSys->GetTexByPath(prefix + "_" + postfixs[i] + ".png");
					iter->mTextures.push_back(texInfo);
					assert(texInfo.texture);
				}
			}
		}
	}
#else
	gModelPath = "cerberus\\"; mModel->LoadModel(MakeModelPath("cerberus.fbx"));;// mScale = 0.1; mPosition = XMFLOAT3(0, 0, 0);

	std::vector<TTexture> textures(4);
	const char* images[] = {
		"cerberus_A.png",
		"cerberus_N.png",
		"cerberus_M.png",
		"cerberus_R.png"
	};
	for (int i = 0; i < 4; ++i) {
		std::string path = images[i];
		textures[i].texture = mRenderSys->_CreateTexture(path.c_str());
		textures[i].path = images[i];
		textures[i].texture->GetDesc(&textures[i].desc);
	}
	for (auto& iter : mModel->mMeshes) {
		iter->mTextures = textures;
	}
	//mModel->mMaterial->mSampler = mRenderSys->CreateSampler(D3D11_FILTER_ANISOTROPIC);
#endif
}

void Lesson6::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
#ifdef USE_RENDER_OP
	mRenderSys->SetWorldTransform(GetWorldTransform());
#else
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
#endif
	mModel->Draw();
}

//auto reg = AppRegister<Lesson6>("TAppLesson6: PBR");