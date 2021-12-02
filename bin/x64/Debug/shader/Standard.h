struct vbSurface
{
    float3 Pos : POSITION;
    float4 Color : COLOR;
	float2 Tex  : TEXCOORD0;
};

cbuffer cbPerLight : register(b1)
{
	matrix LightView;
	matrix LightProjection;	

	float4 unity_LightPosition;	//world space
	float4 unity_LightColor;	//w(gross or luminance), 
	float4 unity_SpecColor;		//w(shiness)
	float4 unity_LightAtten;	//x(cutoff), y(1/(1-cutoff)), z(atten^2)
	float4 unity_SpotDirection;

	int LightType;
	int HasDepthMap;
}

cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	
	matrix WorldInv;
	matrix ViewInv;
	matrix ProjectionInv;

	float4 glstate_lightmodel_ambient;
}

#define clOrange 255,165,0

#if SHADER_MODEL > 30000
SamplerState samLinear : register(s0);
SamplerState samAnsp   : register(s1);
SamplerState samPoint  : register(s2);
#if 0
SamplerState samShadow : register(s3) {
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
    AddressU = Clamp;
    AddressV = Clamp;	
};
#else
SamplerComparisonState samShadow : register(s8) {
   Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
   AddressU = BORDER;
   AddressV = BORDER;
   AddressW = BORDER;
   
   ComparisonFunc = LESS_EQUAL;
};
#endif

Texture2D txMain : register(t0);
Texture2D txDepthMap : register(t8);
TextureCube txSkybox : register(t9);

#define GetTexture2D(TEX, SAMPLER, COORD) TEX.Sample(SAMPLER, COORD)
#define GetTextureCube(TEX, SAMPLER, COORD) TEX.Sample(SAMPLER, COORD)
#define GetTextureCubeLevel(TEX, SAMPLER, COORD, LOD) TEX.SampleLevel(SAMPLER, COORD, LOD)
#else
texture  textureMain : register(t0);
sampler2D txMain : register(s0) =
sampler_state
{
    Texture = <textureMain>;
};

texture  textureDepthMap : register(t8);
sampler2D txDepthMap : register(s8) =
sampler_state
{
    Texture = <textureDepthMap>;
    MinFilter = Point;
    MagFilter = Point;
    MipFilter = Point;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture  textureSkybox : register(t9);
samplerCUBE txSkybox : register(s9) =
sampler_state
{
    Texture = <textureSkybox>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

#define GetTexture2D(TEX, SAMPLER, COORD) tex2D(TEX, COORD)
#define GetTextureCube(TEX, SAMPLER, COORD) texCUBE(TEX, COORD)
#define GetTextureCubeLevel(TEX, SAMPLER, COORD, LOD) texCUBElod(TEX, float4(COORD,LOD)) 
#endif
float4 GetTextureMain(float2 inputTex) {
	return GetTexture2D(txMain, samLinear, inputTex);
}