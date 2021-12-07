inline float3 MirLambertLight(float3 toLight, float3 normal, float3 albedo, bool spotLight)
{
    float3 lightColor = glstate_lightmodel_ambient.xyz;
    {
        float lengthSq = max(dot(toLight, toLight), 0.000001);
        toLight *= rsqrt(lengthSq);

        float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
        if (spotLight)
        {
            float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
            float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
            atten *= saturate(spotAtt);
        }

        float diff = max (0, dot (normal, toLight));
        lightColor += unity_LightColor.rgb * (diff * atten);
    }
    return lightColor * albedo;
}

inline float3 MirBlinnPhongLight(float3 toLight, float3 normal, float3 toEye, float3 albedo, bool spotLight)
{
	//ambient
	float3 lightColor = glstate_lightmodel_ambient.xyz;
    
	//lambert
	float lengthSq = max(dot(toLight, toLight), 0.000001);
	toLight *= rsqrt(lengthSq);
	
	float ndotl = max(0, dot(normal, toLight));
	lightColor += albedo * ndotl * unity_LightColor.rgb;
	
	//blinn-phong
	float3 h = normalize(toLight + toEye);
	float ndoth = max(0, dot(normal, h));
	float spec = pow(ndoth, unity_SpecColor.w*128.0) * unity_LightColor.w;
	//lightColor += spec * unity_SpecColor.rgb;

	//point lighting, spot lighting
	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
	if (spotLight) {
		float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
		float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
		atten *= saturate(spotAtt);
	}
    return lightColor * atten;
}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(MIR_ARGS_SHADOWMAP(shadowMap), float4 shadowPosH)
{
	if (!HasDepthMap) return 1.0;
	
    shadowPosH.xyz /= shadowPosH.w;
	shadowPosH.xy = shadowPosH.xy * 0.5 + 0.5;
#define PCF_SHADOW
#if !defined PCF_SHADOW
	return MIR_SAMPLE_SHADOW(shadowMap, shadowPosH).r;
	//return step(depth, MIR_SAMPLE_SHADOW(shadowMap, shadowPosH).r);
#else
	// 纹素在纹理坐标下的宽高
    const float dx = SMAP_DX;

    float percentLit = 0.0f;
    const float3 offsets[9] = 
	{
        float3(-dx, -dx,  0.0f), float3(0.0f, -dx, 0.0f),  float3(dx, -dx, 0.0f),
		float3(-dx, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(dx, 0.0f, 0.0f),
		float3(-dx, +dx,  0.0f), float3(0.0f, +dx, 0.0f),  float3(dx, +dx, 0.0f)
    };

    // samShadow为compareValue <= sampleValue时为1.0f(反之为0.0f), 对相邻四个纹素进行采样比较
    // 并根据采样点位置进行双线性插值
    // float result0 = depth <= s0;  // .s0      .s1          
    // float result1 = depth <= s1;
    // float result2 = depth <= s2;  //     .depth
    // float result3 = depth <= s3;  // .s2      .s3
    // float result = BilinearLerp(result0, result1, result2, result3, a, b);  // a b为算出的插值相对位置                           
	[unroll]
    for (int i = 0; i < 9; ++i)
    {
		float3 shadowPosOffset = shadowPosH + offsets[i];
        percentLit += MIR_SAMPLE_SHADOW(shadowMap, shadowPosOffset).r;
		//percentLit += step(depth, txDepthMap.Sample(samLinear, shadowPosH.xy + offsets[i]).r);
	}
    
    return percentLit /= 9.0f;
#endif
}