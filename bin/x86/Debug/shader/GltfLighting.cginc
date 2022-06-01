#ifndef GLTF_LIGHTING_H
#define GLTF_LIGHTING_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "LightingInput.cginc"
#include "BRDFCommonFunction.cginc"
#include "ToneMapping.cginc"
#include "IBL.cginc"

MIR_DECLARE_TEX2D(txAlbedo, 0);
MIR_DECLARE_TEX2D(txNormal, 1);
MIR_DECLARE_TEX2D(txMetalness, 2);
MIR_DECLARE_TEX2D(txRoughness, 3);
MIR_DECLARE_TEX2D(txAmbientOcclusion, 4);
MIR_DECLARE_TEX2D(txEmissive, 5);

float3 GetVolumeTransmissionRay(float3 n, float3 v, float thickness, float ior, matrix world)
{
    // Direction of refracted light.
    float3 refractionVector = refract(-v, normalize(n), 1.0 / ior);

    // Compute rotation-independant scaling of the model matrix.
    float3 modelScale;
    modelScale.x = length(float3(world[0].xyz));
    modelScale.y = length(float3(world[1].xyz));
    modelScale.z = length(float3(world[2].xyz));

    // The thickness is specified in local space.
    return normalize(refractionVector) * thickness * modelScale;
}

float ApplyIorToRoughness(float roughness, float ior)
{
    // Scale roughness with IOR so that an IOR of 1.0 results in no microfacet refraction and
    // an IOR of 1.5 results in the default amount of microfacet refraction.
    return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}
float3 GetPunctualRadianceTransmission(float3 normal, float3 view, float3 pointToLight, float roughness,
    float3 f0, float3 f90, float3 baseColor, float ior)
{
    float transmissionRougness = ApplyIorToRoughness(roughness, ior);

    float3 n = normalize(normal);           // Outward direction of surface point
    float3 v = normalize(view);             // Direction from surface point to view
    float3 l = normalize(pointToLight);
    float3 l_mirror = normalize(l + 2.0 * n * dot(-l, n));     // Mirror light reflection vector on surface
    float3 h = normalize(l_mirror + v);            // Halfway vector between transmission light vector and v

    float D = GGXTRDistribution(saturate(dot(n, h)), transmissionRougness);
    float3 F = SchlickFresnel(f0, f90, saturate(dot(v, h)));
    float Vis = SmithJointGGXFilamentVisibility(saturate(dot(n, l_mirror)), saturate(dot(n, v)), transmissionRougness);

    // Transmission BTDF
    return (1.0 - F) * baseColor * D * Vis;
}

float3 ApplyVolumeAttenuation(float3 radiance, float transmissionDistance, float3 attenuationColor, float attenuationDistance)
{
    if (attenuationDistance == 0.0)
    {
        // Attenuation distance is +¡Þ (which we indicate by zero), i.e. the transmitted color is not attenuated at all.
        return radiance;
    }
    else
    {
        // Compute light attenuation using Beer's law.
        float3 attenuationCoefficient = -log(attenuationColor) / attenuationDistance;
        float3 transmittance = exp(-attenuationCoefficient * transmissionDistance); // Beer's law
        return transmittance * radiance;
    }
}
float3 GetTransmissionSample(float2 fragCoord, float roughness, float ior)
{
    float framebufferLod = log2(float(256)) * ApplyIorToRoughness(roughness, ior);
    //float3 transmittedLight = textureLod(u_TransmissionFramebufferSampler, fragCoord.xy, framebufferLod).rgb;
    float3 transmittedLight = MIR_SAMPLE_TEX2D_LEVEL(txMetalness, fragCoord.xy, framebufferLod);
    return transmittedLight;
}
float3 GetIBLVolumeRefraction(float3 n, float3 v, float perceptualRoughness, float3 baseColor, float3 f0, float3 f90,
    float3 worldPos, matrix modelMatrix, matrix viewMatrix, matrix projMatrix, float ior, float thickness, float3 attenuationColor, float attenuationDistance)
{
    float3 transmissionRay = GetVolumeTransmissionRay(n, v, thickness, ior, modelMatrix);
    float3 refractedRayExit = worldPos + transmissionRay;

    // Project refracted vector on the framebuffer, while mapping to normalized device coordinates.
    //float4 ndcPos = projMatrix * viewMatrix * float4(refractedRayExit, 1.0);
    float4 ndcPos = mul(Projection, mul(View, float4(refractedRayExit, 1.0)));
    
    float2 refractionCoords = ndcPos.xy / ndcPos.w;
    refractionCoords = refractionCoords * 0.5 + 0.5;
    refractionCoords.y = 1.0 - refractionCoords.y;

    // Sample framebuffer to get pixel the refracted ray hits.
    float3 transmittedLight = GetTransmissionSample(refractionCoords, perceptualRoughness, ior);

    float3 attenuatedColor = ApplyVolumeAttenuation(transmittedLight, length(transmissionRay), attenuationColor, attenuationDistance);

    // Sample GGX LUT to get the specular component.
    float NdotV = clampedDot(n, v);
    float2 brdfSamplePoint = clamp(float2(NdotV, perceptualRoughness), float2(0.0, 0.0), float2(1.0, 1.0));
    float2 brdf = MIR_SAMPLE_TEX2D(_LUT, brdfSamplePoint).rg;
    float3 specularColor = f0 * brdf.x + f90 * brdf.y;

    return (1.0 - specularColor) * attenuatedColor * baseColor;
}

float3 GltfPbrLight(LightingInput i, float3 l, float3 n, float3 v)
{
    float3 h = normalize(l + v);
    float nl = saturate(dot(n, l));
    float lh = saturate(dot(l, h));
    float nh = saturate(dot(n, h));
    float nv = saturate(dot(n, v));
	float ao = i.ao_rough_metal_tx.x;
    float perceptualRoughness = i.ao_rough_metal_tx.y;
	float metallic = i.ao_rough_metal_tx.z;
    float transmissionFactor = i.ao_rough_metal_tx.w;
    float roughness = perceptualRoughness * perceptualRoughness;
    float3 f0 = lerp(DielectricSpec.rgb, i.albedo.rgb, metallic);
    float3 f90 = float3(1, 1, 1);
    float specularWeight = 1.0;
    
    float3 fcolor = float3(0, 0, 0);

    float3 transmission_color = 0.0;
#if (ENABLE_TRANSMISSION || ENABLE_VOLUME) && (USE_PUNCTUAL || USE_IBL)
    float thickness = 0.0;
    float ior = 1.5;
    float4 attenuationColorDistance = 0.0;   
    transmission_color += transmissionFactor * GetIBLVolumeRefraction(
        n, v,
        perceptualRoughness,
        i.albedo.rgb, f0, f90,
        i.world_pos.xyz, World, View, Projection,
        ior, thickness, attenuationColorDistance.xyz, attenuationColorDistance.w);
#endif

    float3 diffuse_color = 0.0;
    float3 specular_color = 0.0;
#if USE_PUNCTUAL   
    float3 diffuse = LambertDiffuse(i.albedo.rgb * (1.0 - metallic));
    
    float D = GGXTRDistribution(nh, roughness);
    float V = SmithJointGGXFilamentVisibility(nl, nv, roughness);
    float3 F = SchlickFresnel(f0, f90, lh);
    float3 specular = D * V * F;
    
    float3 kd = 1.0 - specularWeight * F;
    float ks = specularWeight;
	diffuse_color  = kd * diffuse * LightColor.rgb * nl;
	specular_color = ks * specular * LightColor.rgb * nl;
    #if ENABLE_TRANSMISSION
        float3 transmissionRay = GetVolumeTransmissionRay(n, v, thickness, ior, World);
        l -= transmissionRay;
        l = normalize(l);

        float3 intensity = 1.0;//getLighIntensity(light, pointToLight);
        float3 transmittedLight = intensity * GetPunctualRadianceTransmission(n, v, l, roughness, f0, f90, i.albedo.rgb, ior);
        transmission_color += transmissionFactor * transmittedLight;
    #endif  
#endif
    
    float3 ibl_diff = 0.0;
    float3 ibl_spec = 0.0;
#if USE_IBL
    ibl_diff = GetIBLRadianceLambertian(n, v, roughness, i.albedo.rgb * (1.0 - metallic), f0, specularWeight);
    ibl_spec = GetIBLRadianceGGX(n, v, perceptualRoughness, f0, specularWeight);    
#endif

    fcolor += lerp(diffuse_color + ibl_diff * ao, transmission_color, transmissionFactor);
    fcolor += specular_color + ibl_spec * ao;
	fcolor += i.emissive.rgb;
    
#if TONEMAP_MODE
	fcolor = toneMap(fcolor, CameraPositionExposure.w);
#endif
    
#if DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    fcolor = linearTosRGB(i.albedo.rgb);   
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_DIFFUSE
	fcolor = kd * diffuse;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR
	fcolor = ks * specular;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_D
	fcolor = float3(D, D, D);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_V
	fcolor = float3(V, V, V);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_F
	fcolor = F;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV
    fcolor = GetIBLRadianceLambertian(n, v, roughness, i.albedo.rgb * (1.0 - metallic), f0, specularWeight); 
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_LUT || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV || DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL	
	fcolor = GetIBLRadianceGGX(n, v, perceptualRoughness, f0, specularWeight); 
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC_ROUGHNESS 
    fcolor = 0.0;
	#if USE_PUNCTUAL
		fcolor += diffuse_color + specular_color;
	#endif
	#if USE_IBL
		fcolor += (ibl_diff + ibl_spec) * ao;
	#endif
	fcolor = linearTosRGB(fcolor);
#elif DEBUG_CHANNEL == DEBUG_TRANSMISSION_VOLUME
    fcolor = linearTosRGB(transmission_color);
#endif
    return fcolor;
}

#endif