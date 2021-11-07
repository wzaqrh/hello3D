#pragma once
#include "core/base/std.h"

namespace mir {

	enum LightType {
		kLightDirectional,
		kLightPoint,
		kLightSpot
	};
	typedef std::shared_ptr<struct cbDirectLight> cbDirectLightPtr;
	typedef std::shared_ptr<struct cbPointLight> cbPointLightPtr;
	typedef std::shared_ptr<struct cbSpotLight> cbSpotLightPtr;
	typedef std::shared_ptr<struct cbGlobalParam> cbGlobalParamPtr;

	typedef std::shared_ptr<struct Pass> PassPtr;
	typedef std::shared_ptr<struct Technique> TechniquePtr;
	typedef std::shared_ptr<struct Material> MaterialPtr;
	typedef std::shared_ptr<struct MaterialFactory> MaterialFactoryPtr;

}