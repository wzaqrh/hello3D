#pragma once
#include "test/framework/app.h"

class GuiDebugWindow {
public:
	void Dispose();
	void Init(mir::Mir* ctx);
	void AddModel(const mir::rend::AssimpModelPtr& model);
	void AddAllCmds();
	void AddRenderingPathSWCmd();
	void AddIBLPunctualSWCmd();
	void AddDebugChannelCmd();
private:
	CoTask<void> UpdateDeferredMtlKeywords(std::string keyword, int value);
	CoTask<void> UpdateMtlsKeywords(std::string keyword, int value);
private:
	std::vector<mir::res::MaterialInstance> mMtls;
	mir::res::MaterialInstance mDeferredMtl;
private:
	mir::Mir* mContext;
	std::vector<mir::rend::AssimpModelPtr> mModels;
	//AddRenderingPathSW
	int mRenderingPath = 0;
	//AddIBLPunctualSW
	bool _USE_IBL = true;
	bool _USE_PUNCTUAL = true;
	//AddDebugChannel
	int _DEBUG_CHANNEL = 0;
};