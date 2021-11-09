#include "core/rendersys/render_system.h"
#include "core/base/utility.h"

namespace mir {

RenderSystem::RenderSystem()
{
}

RenderSystem::~RenderSystem()
{
}

IProgramPtr RenderSystem::CreateProgram(const std::string& name, const char* vsEntry /*= nullptr*/, const char* psEntry /*= nullptr*/)
{
	std::string ext = GetFileExt(name);
	if (ext.empty()) {
		std::string fullname = "shader\\" + mFXCDir + name;
		return CreateProgramByFXC(fullname, vsEntry, psEntry);
	}
	else {
		std::string fullname = "shader\\" + name;
		return CreateProgramByCompile(fullname.c_str(), fullname.c_str(), vsEntry, psEntry);
	}
}

ITexturePtr RenderSystem::LoadTexture(const std::string& __imgPath, DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN*/, bool async/* = true*/, bool isCube/* = false*/)
{
	const char* pSrc = __imgPath.c_str();
	std::string imgPath = __imgPath;
	auto pos = __imgPath.find_last_of("\\");
	if (pos != std::string::npos) {
		imgPath = __imgPath.substr(pos + 1, std::string::npos);
	}

	ITexturePtr texView = nullptr;
	if (mTexByPath.find(imgPath) == mTexByPath.end()) {
		texView = _CreateTexture(imgPath.c_str(), format, async, isCube);
		mTexByPath.insert(std::make_pair(imgPath, texView));
	}
	else {
		texView = mTexByPath[imgPath];
	}
	return texView;
}

}