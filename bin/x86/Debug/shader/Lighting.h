float3 MirLambertLight(float3 toLight, float3 normal, float3 albedo, bool spotLight)
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