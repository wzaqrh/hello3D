#define SHADOWMAPSAMPLER_AND_TEXELSIZE_DEFINED
#define PCF_SHADOW

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

inline float CalcLightAtten(float lengthSq, float3 toLight, bool spotLight) {
	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
	if (spotLight) {
		float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
		float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
		atten *= saturate(spotAtt);
	}	
	return atten;
}

inline float3 GetSpecColor(float3 normal, float3 toEye) 
{
#if ENABLE_ENVIROMENT_MAP
    float3 reflUVW = reflect(-toEye, normal);
	return MIR_SAMPLE_TEXCUBE(_SpecCube, reflUVW).rgb;
#else
	return unity_SpecColor.rgb;
#endif
}

inline float3 MirBlinnPhongLight(float3 toLight_, float3 normal, float3 toEye, float3 albedo, bool spotLight)
{
	//ambient
	float3 luminance = glstate_lightmodel_ambient.xyz;
	
	//lambert
	float lengthSq = max(dot(toLight_, toLight_), 0.000001);
	toLight_ *= rsqrt(lengthSq);
	
	float ndotl = max(0, dot(normal, toLight_));
	luminance += albedo * ndotl * unity_LightColor.rgb;

	//blinn-phong
#if ENABLE_ENVIROMENT_MAP
    float3 reflUVW = reflect(-toEye, normal);
	luminance += MIR_SAMPLE_TEXCUBE(_SpecCube, reflUVW).rgb;
#else
	float3 h = normalize(toLight_ + toEye);
	float ndoth = max(0, dot(normal, h));
	float spec = pow(ndoth, unity_SpecColor.w*128.0) * unity_LightColor.w;
	luminance += spec * unity_SpecColor.rgb;
#endif

	//point lighting, spot lighting
    return luminance * CalcLightAtten(lengthSq, toLight_, spotLight);
}

inline float3 UnityCombineShadowcoordComponents(float2 baseUV, float2 deltaUV, float depth, float3 receiverPlaneDepthBias)
{
    float3 uv = float3(baseUV + deltaUV, depth + receiverPlaneDepthBias.z);
    uv.z += dot(deltaUV, receiverPlaneDepthBias.xy);
    return uv;
}
inline half UnitySampleShadowmap_PCF3x3NoHardwareSupport(float4 coord, float3 receiverPlaneDepthBias)
{
    half shadow = 1;
#ifdef SHADOWMAPSAMPLER_AND_TEXELSIZE_DEFINED
    // when we don't have hardware PCF sampling, then the above 5x5 optimized PCF really does not work.
    // Fallback to a simple 3x3 sampling with averaged results.
    float2 base_uv = coord.xy;
    float2 ts = _ShadowMapTexture_TexelSize.xy;
    shadow = 0;
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(-ts.x, -ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(0, -ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(ts.x, -ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(-ts.x, 0), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(0, 0), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(ts.x, 0), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(-ts.x, ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(0, ts.y), coord.z, receiverPlaneDepthBias));
    shadow += MIR_SAMPLE_SHADOW(_ShadowMapTexture, UnityCombineShadowcoordComponents(base_uv, float2(ts.x, ts.y), coord.z, receiverPlaneDepthBias));
    shadow /= 9.0;
#endif
    return shadow;
}

float CalcShadowFactor(float4 shadowPosH)
{
	if (_ShadowMapTexture_TexelSize.x == 0) return 1.0;
	
    shadowPosH.xyz /= shadowPosH.w;
#if DEBUG_SHADOW_MAP
	return shadowPosH.y;
#endif

#if defined PCF_SHADOW
	return UnitySampleShadowmap_PCF3x3NoHardwareSupport(shadowPosH, float3(0,0,0));
#else
	return MIR_SAMPLE_SHADOW(_ShadowMapTexture, shadowPosH).r;
#endif
}