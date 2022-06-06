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

float3 ApplyVolumeAttenuation(float3 radiance, float transmissionDistance, float4 atten_color_distance)
{
    if (atten_color_distance.w == 0.0) { // Attenuation distance is +¡Þ (which we indicate by zero), i.e. the transmitted color is not attenuated at all.
        return radiance;
    }
    else { // Compute light attenuation using Beer's law.
        float3 attenuationCoefficient = -log(atten_color_distance.xyz) / atten_color_distance.w;
        float3 transmittance = exp(-attenuationCoefficient * transmissionDistance); // Beer's law
        return transmittance * radiance;
    }
}

float3 GetTransmissionSample(float2 fragCoord, float roughness, float ior)
{
    float framebufferLod = ApplyIorToRoughness(roughness, ior) * (LightMapSizeMip.z - 1);
    float3 transmittedLight = MIR_SAMPLE_TEX2D_LEVEL(_LightMap, fragCoord.xy, framebufferLod).rgb;
    return sRGBToLinear(transmittedLight);
}

struct GltfLightInput
{
    float3 h;
    float nl;
    float lh;
    float nh;
    float nv;
    float roughness;
    float3 f0;
    float3 f90;
#if USE_PUNCTUAL || USE_IBL
    float3 diff;
#endif
    float specular_weight;
    float thickness;
    float ior;
    float4 atten_color_distance; 
};
GltfLightInput GetGlftInput(LightingInput i, float3 l, float3 n, float3 v) 
{
    GltfLightInput gli;
    gli.h = normalize(l + v);
    gli.nl = saturate(dot(n, l));
    gli.lh = saturate(dot(l, gli.h));
    gli.nh = saturate(dot(n, gli.h));
    gli.nv = saturate(dot(n, v));
    gli.roughness = i.percertual_roughness * i.percertual_roughness;
    gli.f0 = lerp(DielectricSpec.rgb, i.albedo.rgb, i.metallic);
    gli.f90 = float3(1, 1, 1);
#if USE_PUNCTUAL || USE_IBL
    gli.diff = i.albedo.rgb * (1.0 - i.metallic);
#endif
    gli.specular_weight = 1.0;
    gli.thickness = 0.0;
    gli.ior = 1.5;
    gli.atten_color_distance = 0.0; 
    return gli;
}

float3 GltfLightAdditive(GltfLightInput gli, LightingInput i, float3 fcolor, float3 l, float3 n, float3 v)
{    
    float3 diffuse_color = 0.0;
    float3 specular_color = 0.0;
    float3 transmission_color = 0.0;
    float4 sheen_color_as = float4(0.0, 0.0, 0.0, 1.0);
#if USE_PUNCTUAL   
    float3 diffuse = LambertDiffuse(gli.diff);
    
    float D = GGXTRDistribution(gli.nh, gli.roughness);
    float V = SmithJointGGXFilamentVisibility(gli.nl, gli.nv, gli.roughness);
    float3 F = SchlickFresnel(gli.f0, gli.f90, gli.lh);
    float3 specular = D * V * F;
    
    float3 kd = 1.0 - gli.specular_weight * F;
    float ks = gli.specular_weight;
	diffuse_color  = kd * diffuse * LightColor.rgb * gli.nl;
	specular_color = ks * specular * LightColor.rgb * gli.nl;
    
    #if ENABLE_TRANSMISSION
        // Transmission BTDF
        float3 transmissionRay = GetVolumeTransmissionRay(n, v, gli.thickness, gli.ior, World);
        l -= transmissionRay;
        l = normalize(l);

        float transmissionRougness = ApplyIorToRoughness(gli.roughness, gli.ior);
        float3 l_mirror = normalize(l + 2.0 * n * dot(-l, n));     // Mirror light reflection vector on surface
        float3 h_mirror = normalize(l_mirror + v);            // Halfway vector between transmission light vector and v
        float nh_mirror = saturate(dot(n, h_mirror));
        float vh_mirror = saturate(dot(v, h_mirror));
        float nl_mirror = saturate(dot(n, l_mirror));
    
        float D_mirror = GGXTRDistribution(nh_mirror, transmissionRougness);
        float V_mirror = SmithJointGGXFilamentVisibility(nl_mirror, gli.nv, transmissionRougness);
        float3 F_mirror = SchlickFresnel(gli.f0, gli.f90, vh_mirror);
        transmission_color = i.albedo.rgb * D_mirror * V_mirror * (1.0 - F_mirror) * LightColor.rgb;     
    #endif     
    
    #if ENABLE_SHEEN
        float sheenPerceptualRoughness = max(i.sheen_color_roughness.w, 0.000001); //clamp (0,1]
        float sheenRoughness = sheenPerceptualRoughness * sheenPerceptualRoughness;
    
        float sheenDistribution = CharlieDistribution(sheenRoughness, gli.nh);
        float sheenVisibility = SheenVisibility(gli.nl, gli.nv, sheenRoughness);
        sheen_color_as.rgb = i.sheen_color_roughness.rgb * sheenDistribution * sheenVisibility * LightColor.rgb * gli.nl;
        
        float sheen_metallic = max(max(i.sheen_color_roughness.r, i.sheen_color_roughness.g), i.sheen_color_roughness.b) ;
        float sheen_brdf_nv = MIR_SAMPLE_TEX2D(_LUT, float2(gli.nv, i.sheen_color_roughness.w)).w;
        float sheen_brdf_nl = MIR_SAMPLE_TEX2D(_LUT, float2(gli.nl, i.sheen_color_roughness.w)).w;
        sheen_color_as.a = min(1.0 - sheen_metallic * sheen_brdf_nv, 1.0 - sheen_metallic * sheen_brdf_nl);
    #endif
#endif

 #if ENABLE_TRANSMISSION   
    fcolor += lerp(diffuse_color, transmission_color * i.transmission_factor, i.transmission_factor);
 #else
    fcolor += diffuse_color;
 #endif
    fcolor += specular_color;
#if ENABLE_SHEEN    
    fcolor = fcolor * sheen_color_as.a + sheen_color_as.rgb;
 #endif
    
#if DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_DIFFUSE
	fcolor = kd * diffuse;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR
	fcolor = ks * specular;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_D
	fcolor = D;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_V
	fcolor = V;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_BRDF_SPECULAR_F
	fcolor = F;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC_ROUGHNESS 
	fcolor = diffuse_color + specular_color;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_VOLUME
    fcolor = transmission_color * i.transmission_factor; 
#elif DEBUG_CHANNEL
    fcolor = 0.0;
#endif
    return fcolor;
}

float3 GltfLightBase(GltfLightInput gli, LightingInput i, float3 l, float3 n, float3 v)
{    
 #if ((ENABLE_TRANSMISSION || ENABLE_VOLUME) && USE_PUNCTUAL) || USE_IBL
    float2 lut_uv = saturate(float2(gli.nv, i.percertual_roughness));
    float2 f_ab = MIR_SAMPLE_TEX2D(_LUT, lut_uv).rg;
#endif

    float3 ibl_diff = 0.0;
    float3 ibl_spec = 0.0;
    float3 ibl_sheen = 0.0;
#if USE_IBL
    IBLInput ibl_i = GetIBLInput(n, v, gli.nv, i.percertual_roughness, f_ab, gli.f0, gli.specular_weight);
    ibl_diff = GetIBLRadianceLambertian(ibl_i, n, f_ab, gli.f0, gli.diff, gli.specular_weight);
    ibl_spec = GetIBLRadianceGGX(ibl_i, i.percertual_roughness, gli.specular_weight);
    #if ENABLE_SHEEN
        ibl_sheen = GetIBLRadianceCharlie(ibl_i, gli.nv, i.sheen_color_roughness);
    #endif
#endif

    float3 ibl_transmission = 0.0;
#if (ENABLE_TRANSMISSION || ENABLE_VOLUME) && (USE_PUNCTUAL || USE_IBL)
    float3 transmissionRay = GetVolumeTransmissionRay(n, v, gli.thickness, gli.ior, World);
    float3 exitWorldPos = i.world_pos + transmissionRay;//refracted ray exit pos
    float4 exitNdc = mul(Projection, mul(View, float4(exitWorldPos, 1.0)));// Project refracted vector on the framebuffer, while mapping to normalized device coordinates.
    float2 exitUV = exitNdc.xy / exitNdc.w;
    exitUV = exitUV * float2(0.5, -0.5) + 0.5;
    
    float3 transmittedLight = GetTransmissionSample(exitUV, i.percertual_roughness, gli.ior);// Sample framebuffer to get pixel the refracted ray hits.
    float3 attenuatedColor = ApplyVolumeAttenuation(transmittedLight, length(transmissionRay), gli.atten_color_distance);

    float3 specularColor = gli.f0 * f_ab.x + gli.f90 * f_ab.y;
    ibl_transmission = (1.0 - specularColor) * attenuatedColor * i.albedo.rgb;
    #if DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_REFRACTION_COORDS    
        ibl_transmission = float3(exitUV.x, 1.0 - exitUV.y, 0.0);
    #elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_LIGHT
        ibl_transmission = transmittedLight;
    #elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_ATTEN_COLOR
        ibl_transmission = attenuatedColor;
    #endif 
#endif

    float3 fcolor = 0.0;
 #if ENABLE_TRANSMISSION   
    fcolor += lerp(ibl_diff * i.ao, ibl_transmission * i.transmission_factor, i.transmission_factor);
 #else
    fcolor += ibl_diff * i.ao;
 #endif
    fcolor += ibl_spec * i.ao;
    fcolor += ibl_sheen * i.ao;
	fcolor += i.emissive.rgb;
    
#if DEBUG_CHANNEL == DEBUG_CHANNEL_BASECOLOR
    fcolor = i.albedo.rgb;   
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_LUT
     #if ((ENABLE_TRANSMISSION || ENABLE_VOLUME) && USE_PUNCTUAL) || USE_IBL
	    fcolor = float3(f_ab, 0.0); 
     #else
        fcolor = 0.0;
     #endif
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_DIFFUSE_PREFILTER_ENV
    fcolor = ibl_diff;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV || DEBUG_CHANNEL == DEBUG_CHANNEL_LUT || DEBUG_CHANNEL == DEBUG_CHANNEL_IBL_SPECULAR_PREFILTER_ENV_UV || DEBUG_CHANNEL == DEBUG_CHANNEL_MIP_LEVEL	
    fcolor = ibl_spec;    
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_METTALIC_ROUGHNESS 
	fcolor = (ibl_diff + ibl_spec) * i.ao;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_VOLUME
    fcolor = ibl_transmission * i.transmission_factor;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL || DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_REFRACTION_COORDS || DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_LIGHT || DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_IBL_VOLUME_ATTEN_COLOR
    #if ENABLE_TRANSMISSION
        fcolor = ibl_transmission;
    #else
        clip(-1);
    #endif
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_INTENSITY
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_TRANSMISSION_FACTOR
    fcolor = float3(i.transmission_factor, gli.ior * 0.05, gli.thickness);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_ATTENUATION_COLOR
    fcolor = gli.atten_color_distance.xyz;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_ATTENUATION_DISTANCE_SEPC_WEIGHT
    fcolor = float3(gli.atten_color_distance.w * 0.05, gli.specular_weight * 0.05, 0);
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN || DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL || DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL_LUT_UV || DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL_LOD || DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_IBL_IRRADIANCE
    fcolor = ibl_sheen;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_COLOR
    fcolor = i.sheen_color_roughness.rgb;
#elif DEBUG_CHANNEL == DEBUG_CHANNEL_SHEEN_ROUGHNESS_SCALING
    fcolor = float3(i.sheen_color_roughness.a, 1.0, 1.0);
#elif DEBUG_CHANNEL
    fcolor = 0.0;
#endif
    return fcolor;
}

#endif