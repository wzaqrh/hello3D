#include "Standard.glinc"

struct PixelInput
{
	float4 Color;
	float2 Tex;
};

#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN(float2, iPos, 0);
	MIR_DECLARE_VS_IN(float2, iTex, 1);
	MIR_DECLARE_VS_IN(float4, iColor, 2);
	
	MIR_DECLARE_VS_OUT(PixelInput, o, 0);

	layout (binding = 3, std140) uniform cbImGui 
	{ 
		mat4 ProjectionMatrix; 
	};

	void StageEntry_VS()
	{
		o.Tex = iTex;
		o.Color = iColor;
		gl_Position = ProjectionMatrix * float4(iPos.xy, 0, 1);
	}
#else
	MIR_DECLARE_PS_IN(PixelInput, i, 0);
	MIR_DECLARE_PS_OUT(float4, oColor, 0);

	MIR_DECLARE_TEX2D(_MainTex, 0);

	void StageEntry_PS()
	{
		oColor = i.Color * MIR_SAMPLE_TEX2D(_MainTex, i.Tex);
	}
#endif