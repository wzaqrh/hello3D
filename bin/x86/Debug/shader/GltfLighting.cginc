#ifndef GLTF_LIGHTING_H
#define GLTF_LIGHTING_H
#include "Macros.cginc"
#include "Standard.cginc"
#include "LightingInput.cginc"
#include "BRDFCommonFunction.cginc"
#include "ToneMapping.cginc"
#include "IBL.cginc"


float3 GetVolumeTransmissionRay(float3 n, float3 v, float thickness, float ior, matrix world)
{
    float3 refractionVector = refract(-v, n, 1.0 / ior);// Direction of refracted light.
    float3 modelScale = float3(length(float3(world[0].xyz)), length(float3(world[1].xyz)), length(float3(world[2].xyz)));// Compute rotation-independant scaling of the model matrix.
    return normalize(refractionVector) * thickness * modelScale;// The thickness is specified in local space.
}

float ApplyIorToRoughness(float roughness, float ior)
{
    return roughness * saturate(ior * 2.0 - 2.0);// Scale roughness with IOR so that an IOR of 1.0 results in no microfacet refraction and  an IOR of 1.5 results in the default amount of microfacet refraction.
}

float3 ApplyVolumeAttenuation(float3 radiance, float transmissionDistance, float4 attenuationColorDistance)
{
    if (attenuationColorDistance.w == 0.0) { // Attenuation distance is +¡Þ (which we indicate by zero), i.e. the transmitted color is not attenuated at all.
        return radiance;
    }
    else { // Compute light attenuation using Beer's law.
        float3 attenuationCoefficient = -log(attenuationColorDistance.xyz) / attenuationColorDistance.w;
        float3 transmittance = exp(-attenuationCoefficient * transmissionDistance); // Beer's law
        return transmittance * radiance;
    }
}

float3 GetTransmissionSample(float2 fragCoord, float roughness, float ior)
{
    float framebufferLod = ApplyIorToRoughness(roughness, ior) * (LightMapSizeMip.z - 1);
    float3 transmittedLight = MIR_SAMPLE_TEX2D_LEVEL(_LightMap, fragCoord.xy, framebufferLod);
    return sRGBToLinear(transmittedLight);
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
#if USE_PUNCTUAL || USE_IBL
    float3 diff = i.albedo.rgb * (1.0 - metallic);
#endif
    float specularWeight = 1.0;
    float thickness = 0.0;
    float ior = 1.5;
    float4 attenuationColorDistance = 0.0;      
    
    float3 diffuse_color = 0.0;
    float3 specular_color = 0.0;
#if USE_PUNCTUAL   
    float3 diffuse = LambertDiffuse(diff);
    
    float D = GGXTRDistribution(nh, roughness);
    float V = SmithJointGGXFilamentVisibility(nl, nv, roughness);
    float3 F = SchlickFresnel(f0, f90, lh);
    float3 specular = D * V * F;
    
    float3 kd = 1.0 - specularWeight * F;
    float ks = specularWeight;
	diffuse_color  = kd * diffuse * LightColor.rgb * nl;
	specular_color = ks * specular * LightColor.rgb * nl;
#endif
    
 #if ((ENABLE_TRANSMISSION || ENABLE_VOLUME) && USE_PUNCTUAL) || USE_IBL
    float2 lut_uv = saturate(float2(nv, perceptualRoughness));
    float2 f_ab = MIR_SAMPLE_TEX2D(_LUT, lut_uv).rg;
#endif

    float3 ibl_diff = 0.0;
    float3 ibl_spec = 0.0;
#if USE_IBL
    IBLInput ibl_i = GetIBLInput(perceptualRoughness, nv, f_ab, f0, specularWeight);
    ibl_diff = GetIBLRadianceLambertian(ibl_i, n, f_ab, f0, diff);
    ibl_spec = GetIBLRadianceGGX(ibl_i, n, v, perceptualRoughness);
#endif

    float3 transmission_color = 0.0;
 #if USE_PUNCTUAL && ENABLE_TRANSMISSION
     // Transmission BTDF
     float3 transmissionRay = GetVolumeTransmissionRay(n, v, thickness, ior, World);
     l -= transmissionRay;
     l = normalize(l);

     float transmissionRougness = ApplyIorToRoughness(roughness, ior);
     float3 l_mirror = normalize(l + 2.0 * n * dot(-l, n));     // Mirror light reflection vector on surface
     float3 h_mirror = normalize(l_mirror + v);            // Halfway vector between transmission light vector and v
     float nh_mirror = saturate(dot(n, h_mirror));
     float vh_mirror = saturate(dot(v, h_mirror));
     float nl_mirror = saturate(dot(n, l_mirror));
     
     float D_mirror = GGXTRDistribution(nh_mirror, transmissionRougness);
     float V_mirror = SmithJointGGXFilamentVisibility(nl_mirror, nv, transmissionRougness);
     float3 F_mirror = SchlickFresnel(f0, f90, vh_mirror);
     transmission_color = i.albedo.rgb * D_mirror * V_mirror * (1.0 - F_mirror) * LightColor.rgb;     
 #endif  
    
    float3 ibl_transmission = 0.0;
#if (ENABLE_TRANSMISSION || ENABLE_VOLUME) && (USE_PUNCTUAL || USE_IBL)
    float3 exitWorldPos = i.world_pos + transmissionRay;//refracted ray exit pos
    float4 exitNdc = mul(Projection, mul(View, float4(exitWorldPos, 1.0)));// Project refracted vector on the framebuffer, while mapping to normalized device coordinates.
    float2 exitUV = exitNdc.xy / exitNdc.w;
    exitUV = exitUV * float2(0.5, -0.5) + 0.5;
    
    float3 transmittedLight = GetTransmissionSample(exitUV, perceptualRoughness, ior);// Sample framebuffer to get pixel the refracted ray hits.
    float3 attenuatedColor = ApplyVolumeAttenuation(transmittedLight, length(transmissionRay), attenuationColorDistance);

    float3 specularColor = f0 * f_ab.x + f90 * f_ab.y;
    ibl_transmission = (1.0 - specularColor) * attenuatedColor * i.albedo.rgb;
    #if DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_REFRACTION_COORDS    
        ibl_transmission = float3(exitUV.x, 1.0 - exitUV.y, 0.0);
    #elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_LIGHT
        ibl_transmission = transmittedLight;
    #elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_ATTEN_COLOR
        ibl_transmission = attenuatedColor;
    #endif    
#endif

    float3 fcolor = float3(0, 0, 0);
 #if ENABLE_TRANSMISSION   
    fcolor += lerp(diffuse_color + ibl_diff * ao, (transmission_color + ibl_transmission) * transmissionFactor, transmissionFactor);
 #else
    fcolor += diffuse_color + ibl_diff * ao;
 #endif
    fcolor += specular_color + ibl_spec * ao;
	fcolor += i.emissive.rgb;
    
#if TONEMAP_MODE
	fcolor = toneMap(fcolor, CameraPositionExposure.w);
#endif
    
#if DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    fcolor = linearTosRGB(i.albedo.rgb);   
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_DIFFUSE
	fcolor = any(LightColor.rgb) ? kd * diffuse : MakeDummyColor(v);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR
	fcolor = any(LightColor.rgb) ? ks * specular : MakeDummyColor(v);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_D
	fcolor = any(LightColor.rgb) ? float3(D, D, D) : MakeDummyColor(v);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_V
	fcolor = any(LightColor.rgb) ? float3(V, V, V) : MakeDummyColor(v);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_F
	fcolor = any(LightColor.rgb) ? F : MakeDummyColor(v);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV
    #if USE_IBL
        fcolor = GetIBLRadianceLambertian(ibl_i, n, f_ab, f0, diff);
    #else
        fcolor = MakeDummyColor(v);
    #endif
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_LUT || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV || DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL	
    #if USE_IBL
        fcolor = GetIBLRadianceGGX(ibl_i, n, v, perceptualRoughness);	
    #else
        fcolor = MakeDummyColor(v);
    #endif    
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC_ROUGHNESS 
    fcolor = 0.0;
	#if USE_PUNCTUAL
		fcolor += diffuse_color + specular_color;
	#endif
	#if USE_IBL
		fcolor += (ibl_diff + ibl_spec) * ao;
	#endif
	fcolor = linearTosRGB(fcolor);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_VOLUME
    fcolor = linearTosRGB((transmission_color + ibl_transmission) * transmissionFactor);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL || DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_REFRACTION_COORDS || DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_LIGHT || DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_ATTEN_COLOR
    #if ENABLE_TRANSMISSION
        fcolor = ibl_transmission;
    #else
        clip(-1);
    #endif
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_INTENSITY
    
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_FACTOR
    fcolor = float3(transmissionFactor, ior * 0.05, thickness);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_ATTENUATION_COLOR
    fcolor = attenuationColorDistance.xyz;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_ATTENUATION_DISTANCE_SEPC_WEIGHT
    fcolor = float3(attenuationColorDistance.w * 0.05, specularWeight * 0.05, 0);
#endif
    return fcolor;
}

#endif