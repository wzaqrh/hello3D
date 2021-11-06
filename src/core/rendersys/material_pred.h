#pragma once
#include "core/base/std.h"

namespace mir {

	enum enLightType {
		E_LIGHT_DIRECT,
		E_LIGHT_POINT,
		E_LIGHT_SPOT
	};
	typedef std::shared_ptr<struct cbDirectLight> TDirectLightPtr;
	typedef std::shared_ptr<struct cbPointLight> TPointLightPtr;
	typedef std::shared_ptr<struct cbSpotLight> TSpotLightPtr;

	struct cbGlobalParam;

	struct Pass;
	typedef std::shared_ptr<Pass> TPassPtr;

	struct Technique;
	typedef std::shared_ptr<Technique> TTechniquePtr;

	struct Material;
	typedef std::shared_ptr<Material> TMaterialPtr;

	struct TextureBySlot;
	struct TMaterialBuilder;

	struct MaterialFactory;
	typedef std::shared_ptr<MaterialFactory> TMaterialFactoryPtr;

}