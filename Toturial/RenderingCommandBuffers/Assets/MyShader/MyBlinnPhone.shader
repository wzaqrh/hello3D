Shader "Custom/MyBlinnPhone"
{
    Properties
    {
        _Color ("Color", Color) = (1,1,1,1)
        _MainTex ("Albedo (RGB)", 2D) = "white" {}
        _Ks ("Kspecular", Range(0,1)) = 0.5
        _Kd ("Kdiffuse",  Range(0,1)) = 0.0
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 200

        CGPROGRAM
        // Physically based Standard lighting model, and enable shadows on all light types
        #pragma surface surf BlinnPhong fullforwardshadows
        #pragma target 3.0

        sampler2D _MainTex;

        struct Input
        {
            float2 uv_MainTex;
        };


        UNITY_INSTANCING_BUFFER_START(Props)
		half _Glossiness;
		half _Metallic;
		fixed4 _Color;
        UNITY_INSTANCING_BUFFER_END(Props)

		half4 Lighting_MyBlinnPhong(SurfaceOutput s, half3 viewDir, UnityGI gi)
		{

		}

        void surf (Input IN, inout SurfaceOutputStandard o)
        {
            fixed4 albedo = tex2D (_MainTex, IN.uv_MainTex) * _Color;
            o.Albedo = c.rgb;
            // Metallic and smoothness come from slider variables
            o.Metallic = _Metallic;
            o.Smoothness = _Glossiness;
            o.Alpha = c.a;
        }
        ENDCG
    }
    FallBack "Diffuse"
}
