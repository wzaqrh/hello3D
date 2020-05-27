#pragma once
#include "std.h"

enum enLightType {
	E_LIGHT_DIRECT,
	E_LIGHT_POINT,
	E_LIGHT_SPOT
};
typedef std::shared_ptr<struct TDirectLight> TDirectLightPtr;
typedef std::shared_ptr<struct TPointLight> TPointLightPtr;
typedef std::shared_ptr<struct TSpotLight> TSpotLightPtr;

struct cbGlobalParam;

struct TPass;
typedef std::shared_ptr<TPass> TPassPtr;

struct TTechnique;
typedef std::shared_ptr<TTechnique> TTechniquePtr;

struct TMaterial;
typedef std::shared_ptr<TMaterial> TMaterialPtr;

struct TTextureBySlot;
struct TMaterialBuilder;

struct TMaterialFactory;
typedef std::shared_ptr<TMaterialFactory> TMaterialFactoryPtr;