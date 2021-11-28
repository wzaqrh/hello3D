SET VS_VERSION=vs_4_0
SET PS_VERSION=ps_4_0
SET MACRO=/DSHADER_MODEL=40000
SET OUT=d3d11/

::SET VS_VERSION=vs_3_0
::SET PS_VERSION=ps_3_0
::SET MACRO=/DSHADER_MODEL=30000
::SET OUT=d3d9/

::LayerColor
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo "%OUT%"LayerColor_VS.cso /E VS LayerColor.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo "%OUT%"LayerColor_PS.cso /E PS LayerColor.fx

::Sprite
::fxc  "%MACRO%" /T "%VS_VERSION%" /Fo "%OUT%"Sprite_VS.cso /E VS Sprite.fx
::fxc  "%MACRO%" /T "%PS_VERSION%" /Fo "%OUT%"Sprite_PS.cso /E PS Sprite.fx

::Model
fxc "%MACRO%" /T "%VS_VERSION%" /Fo "%OUT%"Model_VS.cso /E VS Model.fx
fxc "%MACRO%" /T "%PS_VERSION%" /Fo "%OUT%"Model_PS.cso /E PS Model.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo "%OUT%"Model_PSAdd.cso /E PSAdd Model.fx
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo "%OUT%"Model_VSShadowCaster.cso /E VSShadowCaster Model.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo "%OUT%"Model_PSShadowCaster.cso /E PSShadowCaster Model.fx

::ModelPbr
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo ModelPbr_VS.cso /E VS ModelPbr.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ModelPbr_PS.cso /E PS ModelPbr.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ModelPbr_PSAdd.cso /E PSAdd ModelPbr.fx
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo ModelPbr_VSShadowCaster.cso /E VSShadowCaster ModelPbr.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo ModelPbr_PSShadowCaster.cso /E PSShadowCaster ModelPbr.fx

::Skybox
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo "%OUT%"Skybox_VS.cso /E VS Skybox.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo "%OUT%"Skybox_PS.cso /E PS Skybox.fx

::Lesson1
::fxc "%MACRO%" /T "%VS_VERSION%" /Fo "%OUT%"Lesson1_VS.cso /E VS Lesson1.fx
::fxc "%MACRO%" /T "%PS_VERSION%" /Fo "%OUT%"Lesson1_PS.cso /E PS Lesson1.fx

PAUSE