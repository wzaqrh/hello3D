#include <boost/filesystem.hpp>
#include "core/rendersys/render_system.h"

namespace mir {

RenderSystem::RenderSystem()
{
}

RenderSystem::~RenderSystem()
{
}

IProgramPtr RenderSystem::LoadProgram(IResourcePtr res, const std::string& name, const std::string& vsEntry, const std::string& psEntry)
{
	if (res == nullptr) res = CreateResource(kDeviceResourceProgram);

	if (boost::filesystem::path(name).extension().empty()) {
		std::string fullname = "shader\\" + mFXCDir + name;
		return CreateProgramByFXC(res, fullname, vsEntry, psEntry);
	}
	else {
		std::string fullname = "shader\\" + name;
		return CreateProgramByCompile(res, fullname.c_str(), fullname.c_str(), vsEntry, psEntry);
	}
}

ITexturePtr RenderSystem::LoadTexture(IResourcePtr res, const std::string& filepath, 
	ResourceFormat format, bool async, bool isCube)
{
	std::string imgPath = boost::filesystem::system_complete(filepath).string();

	ITexturePtr texView = nullptr;
	if (mTexByPath.find(imgPath) == mTexByPath.end()) {
		texView = _CreateTexture(res, imgPath.c_str(), format, async, isCube);
		mTexByPath.insert(std::make_pair(imgPath, texView));
	}
	else {
		texView = mTexByPath[imgPath];
	}
	return texView;
}

}