#include "TMaterialCB.h"

/********** cbUnityMaterial **********/
cbUnityMaterial::cbUnityMaterial()
{
	_SpecColor = XMFLOAT4(1, 1, 1, 1);
	_Color = XMFLOAT4(1, 1, 1, 1);
	_GlossMapScale = 1;
	_OcclusionStrength = 1;
	_SpecLightOff = 0;
}

/********** cbUnityGlobal **********/
cbUnityGlobal::cbUnityGlobal()
{
	_Unity_IndirectSpecColor = XMFLOAT4(0, 0, 0, 0);
	_AmbientOrLightmapUV = XMFLOAT4(0.01, 0.01, 0.01, 1);
	_Unity_SpecCube0_HDR = XMFLOAT4(0.5, 1, 0, 0);
}