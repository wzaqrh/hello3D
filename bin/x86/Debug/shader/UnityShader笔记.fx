#if defined UnityShader
float3 ShadeVertexLightsFull (float4 vertex, float3 normal, int lightCount, bool spotLight)
{
    float3 viewpos = UnityObjectToViewPos (vertex.xyz);
    float3 viewN = normalize (mul ((float3x3)UNITY_MATRIX_IT_MV, normal));

    float3 lightColor = UNITY_LIGHTMODEL_AMBIENT.xyz;
    for (int i = 0; i < lightCount; i++) {
        float3 toLight = unity_LightPosition[i].xyz - viewpos.xyz * unity_LightPosition[i].w;
        float lengthSq = dot(toLight, toLight);

        // don't produce NaNs if some vertex position overlaps with the light
        lengthSq = max(lengthSq, 0.000001);

        toLight *= rsqrt(lengthSq);

        float atten = 1.0 / (1.0 + lengthSq * unity_LightAtten[i].z);
        if (spotLight)
        {
            float rho = max (0, dot(toLight, unity_SpotDirection[i].xyz));
            float spotAtt = (rho - unity_LightAtten[i].x) * unity_LightAtten[i].y;
            atten *= saturate(spotAtt);
        }

        float diff = max (0, dot (viewN, toLight));
        lightColor += unity_LightColor[i].rgb * (diff * atten);
    }
    return lightColor;
}

"Legacy Shaders/Reflective/VertexLit"
v2f vert(appdata_base v)
{
    v2f o;
    UNITY_SETUP_INSTANCE_ID(v);
    UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
    o.pos = UnityObjectToClipPos(v.vertex);
    o.uv = TRANSFORM_TEX(v.texcoord,_MainTex);
    float4 lighting = float4(ShadeVertexLightsFull(v.vertex, v.normal, 4, true),_ReflectColor.w);
    o.diff = lighting * _Color;
    UNITY_TRANSFER_FOG(o,o.pos);
    return o;
}
fixed4 frag(v2f i) : SV_Target
{
    fixed4 temp = tex2D(_MainTex, i.uv);
    fixed4 c;
    c.xyz = temp.xyz * i.diff.xyz;
    c.w = temp.w * i.diff.w;
    UNITY_APPLY_FOG_COLOR(i.fogCoord, c, fixed4(0,0,0,0)); // fog towards black due to our blend mode
    UNITY_OPAQUE_ALPHA(c.a);
    return c;
}

inline fixed4 UnityBlinnPhongLight (SurfaceOutput s, half3 viewDir, UnityLight light)
{
    half3 h = normalize (light.dir + viewDir);

    fixed diff = max (0, dot (s.Normal, light.dir));

    float nh = max (0, dot (s.Normal, h));
    float spec = pow (nh, s.Specular*128.0) * s.Gloss;

    fixed4 c;
    c.rgb = s.Albedo * light.color * diff + light.color * _SpecColor.rgb * spec;
    c.a = s.Alpha;

    return c;
}
#else
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

inline float3 MirBlinnPhongLight(float3 toLight, float3 toEye, float3 normal, float3 albedo, bool spotLight)
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
#endif