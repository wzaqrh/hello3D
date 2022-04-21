#include "Standard.cginc"
#include "Skeleton.cginc"
#include "Math.cginc"
#include "Lighting.cginc"
#include "LightingPbr.cginc"
#include "ToneMapping.cginc"
#include "Debug.cginc"

#if !defined BOX_KERNEL_SIZE
#define BOX_KERNEL_SIZE 5
#endif
#define KERNEL_LOOP_FIRST (-BOX_KERNEL_SIZE/2)
#define KERNEL_LOOP_LAST  (BOX_KERNEL_SIZE/2)

#define BOX_KERNEL_ALIGN4_COUNT ((BOX_KERNEL_SIZE * BOX_KERNEL_SIZE + 3) / 4)
cbuffer cbBoxFilter : register(b3)
{
	float4 BoxKernelFourWeights[BOX_KERNEL_ALIGN4_COUNT];
};
static float BoxKernelWeights[BOX_KERNEL_ALIGN4_COUNT * 4] = (float[BOX_KERNEL_ALIGN4_COUNT * 4])BoxKernelFourWeights;
	
struct PixelInput
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD0;
};

PixelInput VS(vbSurface input)
{
	PixelInput output;
	output.Pos = float4(input.Pos, 1.0);
	output.Color = input.Color;
	output.Tex = input.Tex;
	return output;
}

float4 PS(PixelInput input) : SV_Target
{
	float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
	for (int row = KERNEL_LOOP_FIRST; row <= KERNEL_LOOP_LAST; row++)
	{
		for (int col = KERNEL_LOOP_FIRST; col <= KERNEL_LOOP_LAST; col++)
		{
			finalColor += MIR_SAMPLE_TEX2D(_SceneImage, input.Tex + float2(float(col), float(row)) * FrameBufferSize.zw)
						* BoxKernelWeights[(row - KERNEL_LOOP_FIRST) * BOX_KERNEL_SIZE + (col - KERNEL_LOOP_FIRST)];
		}
	}
	//if (input.Pos.x < 512) return MIR_SAMPLE_TEX2D(_SceneImage, input.Tex);
	return finalColor;
}