#include "Lesson6.h"

#if 1
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

	auto dl3 = mRenderSys->AddDirectLight();
	dl3->SetDirection(1, -1, 1);
	int intensify = 10;
	dl3->SetDiffuseColor(intensify, intensify, intensify, 1);
#else
	auto light1 = mRenderSys->AddDirectLight();
	light1->SetDirection(-1, 0, 0);

	auto light2 = mRenderSys->AddDirectLight();
	light2->SetDirection(1, 0, 0);

	auto light3 = mRenderSys->AddDirectLight();
	light3->SetDirection(0, 0, 1);
	light3->SetDiffuseColor(3,3,3,1);
#endif

	mModel = new AssimpModel(mRenderSys, "shader\\Lesson6.fx", "shader\\Lesson6.fx");
	
#ifndef PBR_DEBUG
	gModelPath = "Male03\\"; mModel->LoadModel(MakeModelPath("Male03.FBX")); mScale = 0.3; mPosition = XMFLOAT3(0, -5, 0);

	for (auto& iter : mModel->mMeshes) {
		if (!iter->textures.empty()) {
			std::string firstPostfix;
			auto prefix = iter->textures[0].path;
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
				iter->textures.clear();
				for (int i = iter->textures.size(); i < ARRAYSIZE(postfixs); ++i) {
					TextureInfo texInfo = {};
					texInfo.path = prefix + "_" + postfixs[i] + ".png";
					texInfo.texture = mRenderSys->GetTexByPath(texInfo.path);
					iter->textures.push_back(texInfo);
					assert(texInfo.texture);
				}
			}
		}
	}
#else
	gModelPath = "cerberus\\"; mModel->LoadModel(MakeModelPath("cerberus.fbx"));;// mScale = 0.1; mPosition = XMFLOAT3(0, 0, 0);

	std::vector<TextureInfo> textures(4);
	const char* images[] = {
		"cerberus_A.png",
		"cerberus_N.png",
		"cerberus_M.png",
		"cerberus_R.png"
	};
	for (int i = 0; i < 4; ++i) {
		std::string path = images[i];
		textures[i].texture = mRenderSys->CreateTexture(path.c_str());
		textures[i].path = images[i];
		textures[i].texture->GetDesc(&textures[i].desc);
	}
	for (auto& iter : mModel->mMeshes) {
		iter->textures = textures;
	}
#endif

	mModel->mMaterial->mSampler = mRenderSys->CreateSampler(D3D11_FILTER_ANISOTROPIC);
}

void Lesson6::OnRender()
{
	mModel->Update(mTimer.mDeltaTime);
	mRenderSys->ApplyMaterial(mModel->mMaterial, GetWorldTransform());
	mModel->Draw();
}
#else
void Lesson6::OnPostInitDevice()
{
	mLayerColor = std::make_shared<TSprite>(mRenderSys, "shader\\Pbr.fx", "shader\\Pbr.fx");
	mLayerColor->SetPosition(-4, -4, -10);
	mLayerColor->SetSize(8, 8);
}

void Lesson6::OnRender()
{
	mLayerColor->Draw();
}
#endif

auto reg = AppRegister<Lesson6>("TAppLesson6: PBR");