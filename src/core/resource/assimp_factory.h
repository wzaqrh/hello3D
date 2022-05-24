#pragma once
#include <boost/noncopyable.hpp>
#include "core/base/stl.h"
#include "core/base/math.h"
#include "core/base/cppcoro.h"
#include "core/base/launch.h"
#include "core/base/tpl/atomic_map.h"
#include "core/base/declare_macros.h"
#include "core/rendersys/predeclare.h"
#include "core/resource/predeclare.h"

namespace mir {
namespace res {

class AiResourceFactory {
public:
	MIR_MAKE_ALIGNED_OPERATOR_NEW;
	AiResourceFactory(ResourceManager& resMng);

	CoTask<bool> CreateAiScene(AiScenePtr& aiScene, Launch launchMode, std::string assetPath, std::string redirectRes) ThreadSafe;
	DECLARE_COTASK_FUNCTIONS(res::AiScenePtr, CreateAiScene, ThreadSafe);
private:
	CoTask<bool> DoCreateAiScene(AiScenePtr& aiScene, Launch launchMode, std::string assetPath, std::string redirectRes) ThreadSafe;
private:
	ResourceManager& mResMng;
	struct AiResourceKey {
		std::string Path, RedirectResource;
		bool operator<(const AiResourceKey& other) const {
			if (Path != other.Path) return Path < other.Path;
			return RedirectResource < other.RedirectResource;
		}
	};
	tpl::AtomicMap<AiResourceKey, res::AiScenePtr> mAiSceneByKey;
};

}
}