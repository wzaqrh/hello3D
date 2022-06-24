#include "Macros.glinc"
#include "Standard.glinc"
#include "ToneMapping.glinc"

struct PixelInput
{
	float3 Tex;
};

#if SHADER_STAGE == SHADER_STAGE_VERTEX
	MIR_DECLARE_VS_IN(float3, iPos, 0);
	MIR_DECLARE_VS_OUT(PixelInput, o, 0);

	#if DEPRECATE_SKYBOX
	void StageEntry_VS()
	{
		gl_Position = float4(iPos, 1.0);
		o.Tex = normalize(ViewInv * ProjectionInv * gl_Position);
	}
	#else
	void StageEntry_VS()
	{
	#if !RIGHT_HANDNESS_RESOURCE
		o.Tex = iPos;
	#else
		o.Tex = float3(iPos.x, iPos.y, -iPos.z);
	#endif
		gl_Position = View * float4(iPos, 0.0);//没有平移
	#if REVERSE_Z
		gl_Position.xyw = (Projection * gl_Position).xyw;
		gl_Position.z = 0.0;//z永远为0
	#else
		gl_Position = (Projection * gl_Position).xyww;//z永远为1
	#endif
	}
	#endif
#else
	MIR_DECLARE_PS_IN(PixelInput, i, 0);
	MIR_DECLARE_PS_OUT(vec4, oColor, 0);

	MIR_DECLARE_TEXCUBE(_MainTex, 0);

	void StageEntry_PS()
	{	
		oColor.rgb = MIR_SAMPLE_TEXCUBE(_MainTex, i.Tex).rgb;
		oColor.a = 1.0;
	
	#if TONEMAP_MODE
		oColor.rgb = toneMap(oColor.rgb, CameraPositionExposure.w);
	#endif
	}
#endif