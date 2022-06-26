#include "Standard.glinc"

struct PixelInput
{
	float4 Color;
};

#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN_Surface(i, 0);
	MIR_DECLARE_VS_OUT(PixelInput, o, 0);

	void StageEntry_VS()
	{
		matrix WVP = World * View * Projection;
		gl_Position = float4(iPos,1.0) * WVP;
		o.Color = iColor;
	}
#else
	MIR_DECLARE_PS_IN(PixelInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	void StageEntry_PS()
	{	
		oColor = i.Color;
	}
#endif