<Shader Name="Standard">
	<PROGRAM>		
		<PredMacros>Macros</PredMacros>
		
		<Attribute Name="vbSurface">
			<Element SemanticName="POSITION" Format="rgb32f" SemanticIndex="0"></Element>
			<Element SemanticName="COLOR" 	 Format="rgba8un"></Element>
			<Element SemanticName="TEXCOORD" Format="rg32f"></Element>
		</Attribute>
		
		<Uniform Name="cbPerFrame" Slot="0" ShareMode="PerFrame">
			<Element Name="World" 		  		Type="matrix" Count="0" Offset="0" Default="1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1"></Element>
			<Element Name="View" 	 			Type="matrix"></Element>
			<Element Name="Projection" 			Type="matrix"></Element>
			<Element Name="ViewInv" 			Type="matrix"></Element>
			<Element Name="ProjectionInv"		Type="matrix"></Element>
			
			<Element Name="LightView" 	 		Type="matrix"></Element>
			<Element Name="LightProjection" 	Type="matrix"></Element>

			<Element Name="CameraPositionExposure"	Type="float4"></Element>
			
			<Element Name="SHC0C1"				Type="matrix"></Element>
			<Element Name="SHC2"				Type="matrix"></Element>
			<Element Name="SHC2_2"				Type="float4"></Element>
			<Element Name="EnvDiffuseColor"		Type="float4"></Element>
			<Element Name="EnvSpecColorMip"		Type="float4"></Element>
			<Element Name="EnvSheenColorMip"	Type="float4"></Element>
			<Element Name="LightMapUV"			Type="float4"></Element>
			<Element Name="LightMapSizeMip"		Type="float4"></Element>
			
			<Element Name="FrameBufferSize"		Type="float4"></Element>
			<Element Name="ShadowMapSize"		Type="float4"></Element>
		</Uniform>
		<Uniform Name="cbPerLight" Slot="1" ShareMode="PerFrame">
			<Element Name="LightPosition"		Type="float4"></Element>
			<Element Name="LightColor"			Type="float4"></Element> 
			<Element Name="unity_LightAtten"	Type="float4"></Element>
			<Element Name="unity_SpotDirection"	Type="float4"></Element>
			<Element Name="LightRadiusUVNearFar" Type="float4"></Element>
			<Element Name="LightDepthParam"		Type="float4"></Element>
			<Element Name="IsSpotLight" 	 	Type="bool"></Element>
		</Uniform>
		
		<Texture Name="tGrpShadowMap">
			<Element Slot="11" CompFunc="Less" AddressU="Border" AddressV="Border" AddressW="Wrap" >_ShadowMap</Element>
			<Element Slot="11" Filter="MinMagMipLinear" AddressU="Clamp" AddressV="Clamp" AddressW="Wrap" Condition="SHADOW_MODE==5">_ShadowMap</Element>
		</Texture>
		<Texture Name="tGrpEnv">
			<Element Slot="12" Filter="MinMagMipLinear" AddressU="Clamp" AddressV="Clamp" AddressW="Wrap">_EnvSheenMap</Element>
			<Element Slot="13" Filter="MinMagMipLinear" AddressU="Clamp" AddressV="Clamp" AddressW="Wrap">_EnvDiffuseMap</Element>
			<Element Slot="14" Filter="MinMagMipLinear" AddressU="Clamp" AddressV="Clamp" AddressW="Wrap">_EnvSpecMap</Element>
			<Element Slot="15" Filter="MinMagMipLinear">_LUT</Element>
		</Texture>
		<Texture Name="tGrpGBuffer">
			<Element Slot="0" Filter="Point">_GDepth</Element>
			<Element Slot="1" Filter="MinMagMipLinear">_GBufferPos</Element>
			<Element Slot="2" Filter="MinMagMipLinear">_GBufferNormal</Element>
			<Element Slot="3" Filter="MinMagMipLinear">_GBufferAlbedo</Element>
			<Element Slot="4" Filter="MinMagMipLinear">_GBufferEmissive</Element>
			<Element Slot="5" Filter="MinMagMipLinear">_GBufferSheen</Element>
			<Element Slot="6" Filter="MinMagMipLinear">_GBufferClearCoat</Element>
		</Texture>
		<Texture Name="tGrpLightMap">
			<Element Slot="8" Filter="MinMagMipLinear" AddressU="Clamp" AddressV="Clamp" AddressW="Wrap">_LightMap</Element>
		</Texture>
		<Texture Name="tGrpPostProcessInput">
			<Element Slot="8" Filter="MinMagMipLinear">_SceneImage</Element>
		</Texture>
		
		<Texture Name="tGrpForward">
			<UseTexture>tGrpLightMap</UseTexture>
			<UseTexture>tGrpShadowMap</UseTexture>
			<UseTexture>tGrpEnv</UseTexture>
		</Texture>
		<Texture Name="tGrpPrepassFinal">
			<UseTexture>tGrpGBuffer</UseTexture>
			<UseTexture>tGrpLightMap</UseTexture>
			<UseTexture>tGrpShadowMap</UseTexture>
			<UseTexture>tGrpEnv</UseTexture>
		</Texture>
		<Texture Name="tGrpPostProc">
			<UseTexture>tGrpGBuffer</UseTexture>
			<UseTexture>tGrpPostProcessInput</UseTexture>
		</Texture>
	</PROGRAM>
</Shader>