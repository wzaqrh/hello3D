::Fxc.exe /Od /Zi /E PS /T ps_4_0 /Fo LayerColor_PS.fxc LayerColor.fx
fxc.exe /E VS /T vs_4_0_level_9_3 /Fo LayerColor_VS.fxc LayerColor.fx
::Fxc.exe /nologo /E main /T vs_5_1 /Fo ParticleVS.cso ParticleVS.hlsl

::Fxc.exe /Od /Zi /E VS /T vs_3_0 /Fo ModelPbr_VS.fxc ModelPbr.fx
::Fxc.exe /E PS /T ps_4_0 /Fo ModelPbr_PS.fxc ModelPbr.fx
::Fxc.exe /E PSAdd /T ps_4_0 /Fo ModelPbr_PSAdd.fxc ModelPbr.fx
::Fxc.exe /E VSShadowCaster /T vs_4_0 /Fo ModelPbr_VSShadowCaster.fxc ModelPbr.fx
::Fxc.exe /E PSShadowCaster /T ps_4_0 /Fo ModelPbr_PSShadowCaster.fxc ModelPbr.fx

PAUSE