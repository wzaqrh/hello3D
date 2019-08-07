#include "Lesson6.h"
#include "TMaterialCB.h"
#include "IRenderable.h"
#include "TInterfaceType.h"

void Lesson6::OnInitLight()
{
#if 0
	{
		auto light = mRenderSys->AddPointLight();//1, -1, 1
		float ddd = 10;
		light->SetPosition(-ddd, ddd, -ddd);
		light->SetAttenuation(1, 0.001, 0);
	}
#endif

#if 0
	{
		auto light = mRenderSys->AddDirectLight();
		light->SetDirection(0, -1, 0);
		float i = 0.7;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
#endif

#if 1
	{
		auto light = mRenderSys->AddDirectLight();
		light->SetDirection(1, -1, 1);
		float i = 0.7;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
	{
		auto light = mRenderSys->AddDirectLight();
		light->SetDirection(-1, -1, 0);
		float i = 1.0;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
#endif

#if 0
	{
		auto light = mRenderSys->AddDirectLight();
		light->SetDirection(1, -1, 1);
		float i = 1.0;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
	{
		auto light = mRenderSys->AddDirectLight();
		light->SetDirection(-1, -1, -1);
		float i = 1.0;
		light->SetDiffuseColor(i, i, i, 1.0);
	}
#endif
}

//#define PBR_DEBUG
/********** Lesson6 **********/
void Lesson6::OnPostInitDevice()
{
	mRenderSys->SetSkyBox("images\\uffizi_cross.dds");
#if 0
	//mRenderSys->_CreatePSByFXC("shader\\ModelPbr_PS.cso");
	//mRenderSys->_CreateVS("shader\\LayerColor.fx", nullptr, false);
	//mRenderSys->_CreateVSByFXC("shader\\LayerColor_VS.cso");
	//mRenderSys->_CreateVSByFXC("shader\\Model_VS.cso");
	//mRenderSys->_CreateVSByFXC("shader\\ModelPbr_VS.cso");
#else
	TIME_PROFILE(Lesson6_OnPostInitDevice);

	mRenderSys->SetCamera(45, 30, 1000);
	{
		TIME_PROFILE(SetSkyBox);
		//mRenderSys->SetSkyBox("images\\uffizi_cross.dds");
	}

	mModel = new AssimpModel(mRenderSys, mMove, E_MAT_MODEL_PBR);
	//auto fileName = "Male02.FBX";//99ms,4688ms
	auto fileName = "Male02.assbin";//37ms,2823ms
	//auto fileName = "Male02.dae";//37ms,2823ms
	gModelPath = "Male03\\"; mModel->LoadModel(MakeModelPath(fileName)); mMove->SetDefScale(0.07); mMove->SetPosition(0, -5, 0);

	for (auto& iter : mModel->mMeshes) {
		if (!iter->mTextures->empty() && (*iter->mTextures)[0]) {
			std::string firstPostfix;
			auto prefix = ((*iter->mTextures)[0])->GetPath();
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
				iter->mTextures->clear();
				for (int i = iter->mTextures->size(); i < ARRAYSIZE(postfixs); ++i) {
					TTexturePtr texInfo = mRenderSys->GetTexByPath(prefix + "_" + postfixs[i] + ".png");
					iter->mTextures->push_back(texInfo);
					//assert(texInfo->texture);
				}
			}
		}
	}
#endif
}

void Lesson6::OnRender()
{
	if (mModel) mModel->Update(mTimer.mDeltaTime);
	if (mRenderSys->BeginScene()) {
		TRenderOperationQueue opQueue;
		if (mModel) mModel->GenRenderOperation(opQueue);

		//mRenderSys->RenderQueue(opQueue, E_PASS_SHADOWCASTER);
		mRenderSys->RenderQueue(opQueue, E_PASS_FORWARDBASE);

		mRenderSys->EndScene();
	}
}

auto reg = AppRegister<Lesson6>("TAppLesson6: PBR");