#pragma once
#include "test/framework/app.h"

class GuiDebugWindow {
public:
	void Dispose();
	void Init(mir::Mir* ctx);
	void AddModel(const mir::rend::AssimpModelPtr& model);
	void AddPostProcessEffect(const mir::rend::PostProcessPtr& effect);
	void AddAllCmds();
	void AddRenderBackendSWCmd();
	void AddRenderingPathSWCmd();
	void AddIBLPunctualSWCmd();
	void AddDebugChannelCmd();
	void AddSSAOCmd();
	void AddPostProcessCmd();
private:
	CoTask<void> UpdateDeferredMtlKeywords(std::string keyword, int value);
	CoTask<void> UpdateMtlsKeywords(std::string keyword, int value);
	CoTask<void> UpdateEffectMtlsKeywords(std::string keyword, int value);
private:
	std::vector<mir::res::MaterialInstance> mMtls, mEffectMtls;
	mir::res::MaterialInstance mDeferredMtl;
private:
	mir::Mir* mContext;
	std::vector<mir::rend::AssimpModelPtr> mModels;
	std::vector<mir::rend::PostProcessPtr> mEffects;
	//AddRenderBackendSWCmd
	int mIsOpenglBackend = 1;
	//AddRenderingPathSW
	int mRenderingPath = 0;
	//AddIBLPunctualSW
	bool _USE_IBL = true;
	bool _USE_PUNCTUAL = true;
	//AddDebugChannel
	int _DEBUG_CHANNEL = 0;
	//AddPostProcessCmd
	bool mEnablePostProcEffects = true;
	int _DEBUG_SSAO_CHANNEL = 0;
};