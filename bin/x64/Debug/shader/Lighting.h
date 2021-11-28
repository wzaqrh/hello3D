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
	lightColor += spec * unity_SpecColor.rgb;

	//point lighting, spot lighting
	float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten.z);
	if (spotLight) {
		float rho = max (0, dot(toLight, unity_SpotDirection.xyz));
		float spotAtt = (rho - unity_LightAtten.x) * unity_LightAtten.y;
		atten *= saturate(spotAtt);
	}
    return lightColor * atten;
}