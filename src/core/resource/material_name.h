#pragma once
#include "core/mir_config_macros.h"

#define MAT_SPRITE "Sprite"
#define MAT_LAYERCOLOR "LayerColor"
#define MAT_LABEL "Label"
#define MAT_SKYBOX "Skybox"
#define MAT_MODEL "Model"
#define MAT_MODEL_SHADOW "ModelShadow"
#define MAT_BLOOM "Bloom"
#define MAT_BOX_BLUR "BoxBlur"
#define MAT_SSAO "HorizonBasedAO"
#define MAT_PAINT "Paint3D"
#define MAT_LINE_PAINT "LinePaint3D"

#define LIGHTMODE_SHADOW_CASTER_ "ShadowCaster"
#define LIGHTMODE_FORWARD_BASE_ "ForwardBase"
#define LIGHTMODE_FORWARD_ADD_ "ForwardAdd"
#define LIGHTMODE_PREPASS_BASE_ "PrepassBase"
#define LIGHTMODE_PREPASS_FINAL_ "PrepassFinal"
#define LIGHTMODE_TRANSPARENT_ "Transparent"
#define LIGHTMODE_SKYBOX_ "Skybox"
#define LIGHTMODE_OVERLAY_ "Overlay"
#define LIGHTMODE_POSTPROCESS_ "PostProcess"
inline int GetLightModeValueByName(const std::string& lightModeName)
{
	if (lightModeName == LIGHTMODE_SHADOW_CASTER_) return LIGHTMODE_SHADOW_CASTER;
	if (lightModeName == LIGHTMODE_FORWARD_BASE_) return LIGHTMODE_FORWARD_BASE;
	if (lightModeName == LIGHTMODE_FORWARD_ADD_) return LIGHTMODE_FORWARD_ADD;
	if (lightModeName == LIGHTMODE_PREPASS_BASE_) return LIGHTMODE_PREPASS_BASE;
	if (lightModeName == LIGHTMODE_PREPASS_FINAL_) return LIGHTMODE_PREPASS_FINAL;
	if (lightModeName == LIGHTMODE_TRANSPARENT_) return LIGHTMODE_TRANSPARENT;
	if (lightModeName == LIGHTMODE_SKYBOX_) return LIGHTMODE_SKYBOX;
	if (lightModeName == LIGHTMODE_OVERLAY_) return LIGHTMODE_OVERLAY;
	if (lightModeName == LIGHTMODE_POSTPROCESS_) return LIGHTMODE_POSTPROCESS;
	return -1;
}