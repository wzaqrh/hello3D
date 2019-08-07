SET VS_VERSION=vs_5_0
SET PS_VERSION=ps_4_0

::LayerColor
fxc /T "%VS_VERSION%" /Fo LayerColor_VS.cso /E VS LayerColor.fx
fxc /T "%PS_VERSION%" /Fo LayerColor_PS.cso /E PS LayerColor.fx

::Sprite
fxc /T "%VS_VERSION%" /Fo Sprite_VS.cso /E VS Sprite.fx
fxc /T "%PS_VERSION%" /Fo Sprite_PS.cso /E PS Sprite.fx

::Model
fxc /T "%VS_VERSION%" /Fo Model_VS.fxc /E VS Model.fx
fxc /T "%PS_VERSION%" /Fo Model_PS.cso /E PS Model.fx

::ModelPbr
fxc /T "%VS_VERSION%" /Fo ModelPbr_VS.cso /E VS ModelPbr.fx
fxc /T "%PS_VERSION%" /Fo ModelPbr_PS.cso /E PS ModelPbr.fx
fxc /T "%PS_VERSION%" /Fo ModelPbr_PSAdd.cso /E PSAdd ModelPbr.fx
fxc /T "%VS_VERSION%" /Fo ModelPbr_VSShadowCaster.cso /E VSShadowCaster ModelPbr.fx
fxc /T "%PS_VERSION%" /Fo ModelPbr_PSShadowCaster.cso /E PSShadowCaster ModelPbr.fx

::ShadowMap
fxc /T "%VS_VERSION%" /Fo ShadowMap_VS.fxc /E VS ShadowMap.fx
fxc /T "%PS_VERSION%" /Fo ShadowMap_PS.cso /E PS ShadowMap.fx

::ShadowMap
fxc /T "%VS_VERSION%" /Fo ShadowDepth_VS.fxc /E VS ShadowDepth.fx
fxc /T "%PS_VERSION%" /Fo ShadowDepth_PS.cso /E PS ShadowDepth.fx

::Skybox
fxc /T "%VS_VERSION%" /Fo Skybox_VS.cso /E VS Skybox.fx
fxc /T "%PS_VERSION%" /Fo Skybox_PS.cso /E PS Skybox.fx


PAUSE