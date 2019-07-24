/********** Multi Light(Direct Point) (world space) (SpecularMap) **********/
SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);
Texture2D txSpecular : register(t1);
Texture2D txNormal : register(t2);

struct LIGHT_DIRECT
{
	float4 LightPos;//world space
	float4 DiffuseColor;
	float4 SpecularColorPower;
};

struct LIGHT_POINT
{
	float4 LightPos;//world space
	float4 DiffuseColor;
	float4 SpecularColorPower;
	float4 Attenuation;
};

static const int MAX_LIGHTS = 4;
cbuffer cbGlobalParam : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix ViewInv;
	int4 LightNum;
	LIGHT_DIRECT DirectLights[MAX_LIGHTS];
	LIGHT_POINT PointLights[MAX_LIGHTS];
}

static const int MAX_MATRICES = 256;
cbuffer cbWeightedSkin : register(b1)
{
	matrix Model;
	matrix Models[MAX_MATRICES] : WORLDMATRIXARRAY;	
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
	float3 Tangent : NORMAL1;
	float2 Tex  : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    uint4  BlendIndices : BLENDINDICES;
	float3 BiTangent : NORMAL2;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Normal : NORMAL;//world space
	float3 Eye : POSITION0;//world space
	float3 PointLights[MAX_LIGHTS] : POSITION1;//world space
	float3 DirectLights[MAX_LIGHTS] : POSITION5;//world space
};

float4 Skinning(float4 iBlendWeights, uint4 iBlendIndices, float4 iPos)
{
    float BlendWeights[4] = (float[4])iBlendWeights;
	BlendWeights[3] = 1.0 - BlendWeights[0] - BlendWeights[1] - BlendWeights[2];
    uint  BlendIndices[4] = (uint[4])iBlendIndices;	
	
    float4	Pos = float4(0.0,0.0,0.0,iPos.w);   
	const int NumBones = 4;
    for (int iBone = 0; iBone < NumBones; iBone++) {
		uint Indice = BlendIndices[iBone];
		float Weight = BlendWeights[iBone];
			
		float4 bonePos = mul(iPos, Models[Indice]);
        Pos.xyz += bonePos.xyz * Weight;
    }
	return Pos;
}

PS_INPUT VS(VS_INPUT i)
{
	PS_INPUT output = (PS_INPUT)0;

	matrix MW = mul(World, transpose(Model));
	matrix MWV = mul(View, MW);
	
	output.Normal = normalize(mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Normal.xyz, 0.0))).xyz);
	output.Pos = mul(MW, Skinning(i.BlendWeights, i.BlendIndices, float4(i.Pos.xyz, 1.0)));	
	for (int j = 0; j < LightNum.x; ++j) {
		output.DirectLights[j] = normalize(-DirectLights[j].LightPos.xyz);	
	}
	for (int j = 0; j < LightNum.y; ++j) {
		output.PointLights[j] = normalize(PointLights[j].LightPos.xyz - output.Pos.xyz);
	}
	
	output.Pos = mul(View, output.Pos);
	output.Eye = mul(ViewInv, float4(0.0,0.0,0.0,1.0)) - output.Pos;
	
    output.Pos = mul(Projection, output.Pos);
    
	output.Tex = i.Tex;
    return output;
}

float3 GetDiffuseBySampler(float3 normal, float3 light, float3 lightDiffuseColor, float2 texcoord) {
	float diffuseFactor = saturate(dot(normal, light));
	float3 diffuseMat = txDiffuse.Sample(samLinear, texcoord).xyz;
	return diffuseMat * diffuseFactor * lightDiffuseColor;
}

float3 GetSpecularByDef(float3 normal, float3 light, float3 eye, float4 SpecColorPower) {
	float3 reflection = reflect(-light, normal);
	float specularFactor = saturate(dot(reflection, eye));
	specularFactor = pow(specularFactor, SpecColorPower.w);
	return specularFactor * SpecColorPower.xyz;
}
float3 GetSpecularBySampler(float3 normal, float3 light, float3 eye, float4 SpecColorPower, float2 texcoord) {
	float3 specularMat = txSpecular.Sample(samLinear, texcoord).xyz;
	return GetSpecularByDef(normal, light, eye, SpecColorPower) * specularMat;
}

float3 CalPointLight(float3 normal, float3 light, float3 eye, float2 texcoord, float4 DiffuseColor, float4 SpecularColorPower) {
	float3 color = 0.0;
	if (dot(normal, light) > 0.0) 
	{
		float3 diffuse = GetDiffuseBySampler(normal, light, DiffuseColor.xyz, texcoord);
		float3 specular = GetSpecularBySampler(normal, light, eye, SpecularColorPower, texcoord);
		color = diffuse + specular;	
	}
	return color;
}

float4 PS(PS_INPUT input) : SV_Target
{	
	float3 normal = normalize(input.Normal);
	float3 eye = normalize(input.Eye);
	
	float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < LightNum.x; ++i) {
		float3 light = normalize(input.DirectLights[i]);
		finalColor.xyz += CalPointLight(normal, light, eye, input.Tex, DirectLights[i].DiffuseColor, DirectLights[i].SpecularColorPower);
	}
	for (int i = 0; i < LightNum.y; ++i) {
		float3 light = normalize(input.PointLights[i]);
		finalColor.xyz += CalPointLight(normal, light, eye, input.Tex, PointLights[i].DiffuseColor, PointLights[i].SpecularColorPower);
	}
	
	//float dir = -eye.z;
	//float dir = -normal.z;
	//float dir = -input.PointLights[0].z;
	//finalColor = float4(dir, dir, dir, 1.0);
	
	//finalColor = float4(reflect(-input.PointLights[0], normal), 1.0);
	//float a = dot(input.PointLights[0], normal);
	//finalColor = float4(a, a, a, 1.0);
	
	//finalColor = float4(normal.x, normal.y, -normal.z, 1.0);
	//finalColor = float4(input.PointLights[0].x, input.PointLights[0].y, -input.PointLights[0].z, 1.0);
	
	return finalColor;
}



