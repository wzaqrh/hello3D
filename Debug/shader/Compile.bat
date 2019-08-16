::SET VS_VERSION=vs_5_0
::SET PS_VERSION=ps_4_0
SET VS_VERSION=vs_3_0
SET PS_VERSION=ps_3_0
SET MACRO=/DSHADER_MODEL=30000

::LayerColor
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo LayerColor_VS.cso /E VS LayerColor.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo LayerColor_PS.cso /E PS LayerColor.fx

::Sprite
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo Sprite_VS.cso /E VS Sprite.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo Sprite_PS.cso /E PS Sprite.fx

::Model
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo Model_VS.cso /E VS Model.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo Model_PS.cso /E PS Model.fx

::ModelPbr
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo ModelPbr_VS.cso /E VS ModelPbr.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ModelPbr_PS.cso /E PS ModelPbr.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ModelPbr_PSAdd.cso /E PSAdd ModelPbr.fx
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo ModelPbr_VSShadowCaster.cso /E VSShadowCaster ModelPbr.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ModelPbr_PSShadowCaster.cso /E PSShadowCaster ModelPbr.fx

::ShadowMap
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo ShadowMap_VS.fxc /E VS ShadowMap.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ShadowMap_PS.cso /E PS ShadowMap.fx

::ShadowMap
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo ShadowDepth_VS.fxc /E VS ShadowDepth.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ShadowDepth_PS.cso /E PS ShadowDepth.fx

::Skybox
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo Skybox_VS.cso /E VS Skybox.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo Skybox_PS.cso /E PS Skybox.fx

::Lesson1
fxc "%MACRO%" /T "%VS_VERSION%" /Fo Lesson1_VS.cso /E VS Lesson1.fx
fxc "%MACRO%" /T "%PS_VERSION%" /Fo Lesson1_PS.cso /E PS Lesson1.fx

PAUSE