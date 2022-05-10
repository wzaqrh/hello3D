<Shader Name="Model">
	<Include>Standard</Include>
	<Include>Skeleton</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseAttribute>vbWeightedSkin</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseUniform>cbWeightedSkin</UseUniform>
		<UseTexture>txGlobal</UseTexture>
		<Uniform Name="cbModel" Slot="3" ShareMode="1">
			<Element Name="AlbedoUV"              Type="float4" Default="0,0,1,1"></Element>
			<Element Name="NormalUV"              Type="float4" Default="0,0,1,1"></Element>
			<Element Name="OcclusionUV"           Type="float4" Default="0,0,1,1"></Element>
			<Element Name="RoughnessUV"           Type="float4" Default="0,0,1,1"></Element>
			<Element Name="MetallicUV"            Type="float4" Default="0,0,1,1"></Element>
			<Element Name="EmissiveUV"            Type="float4" Default="0,0,1,1"></Element>
			
			<Element Name="AlbedoFactor"            Type="float4" Default="1,1,1,1"></Element>
			<Element Name="NormalScale"             Type="float"  Default="1"></Element>
			<Element Name="OcclusionStrength"       Type="float"  Default="1"></Element>
			<Element Name="RoughnessFactor"         Type="float"  Default="1"></Element>
			<Element Name="MetallicFactor"          Type="float"  Default="1"></Element>
			<Element Name="EmissiveFactor"          Type="float3"></Element>
			
			<Element Name="EnableAlbedoMap"           Type="bool"></Element>
			<Element Name="EnableNormalMap"           Type="bool"></Element>
			<Element Name="EnableAmbientOcclusionMap" Type="bool"></Element>
			<Element Name="EnableRoughnessMap"        Type="bool"></Element>
			<Element Name="EnableMetalnessMap"        Type="bool"></Element>
			<Element Name="EnableEmissiveMap"         Type="bool"></Element>
			<Element Name="AmbientOcclusion_ChannelGRoughness_ChannelBMetalness" Type="bool"></Element>
			<Element Name="AlbedoMapSRGB"             Type="bool"  Default="1"></Element>
			<Element Name="EmissiveMapSRGB"           Type="bool"  Default="1"></Element>
			<Element Name="HasTangent"                Type="bool"></Element>
		</Uniform>   
		<Texture>
			<Element Slot="0" Address="1">txAlbedo</Element>
			<Element Slot="1" Address="1">txNormal</Element>
			<Element Slot="2" Address="1">txMetalness</Element>
			<Element Slot="3" Address="1">txRoughness</Element>
			<Element Slot="4" Address="1">txAmbientOcclusion</Element>
			<Element Slot="5" Address="1">txEmissive</Element>
		</Texture>
		<Macros>
		</Macros>
		<FileName>Model</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<Tags> <LightMode>ShadowCaster</LightMode> </Tags>
			<PROGRAM> 
				<VertexEntry>VSShadowCaster</VertexEntry> 
				<PixelEntry>PSShadowCasterDebug</PixelEntry>
			</PROGRAM>
			<PROGRAM Condition="SHADOW_MODE==5"> 
				<VertexEntry>VSGenerateVSM</VertexEntry> 
				<PixelEntry>PSGenerateVSM</PixelEntry>
			</PROGRAM>
		</Pass>
		<Pass>
			<UseGrab>_ShadowMap{bind_slot:14}</UseGrab>
			<Tags> <LightMode>ShadowCaster+1</LightMode> </Tags>
			<PROGRAM>
				<VertexEntry>VSBlurVSM</VertexEntry>
				<PixelEntry>PSBlurVSMX</PixelEntry>
			</PROGRAM>
			<GrabPass>VSM_BlurX{fmts:[16,45]}</GrabPass>
		</Pass>
		<Pass>
			<UseGrab>VSM_BlurX{bind_slot:14}</UseGrab>
			<Tags> <LightMode>ShadowCaster+1</LightMode> </Tags>
			<PROGRAM>
				<VertexEntry>VSBlurVSM</VertexEntry>
				<PixelEntry>PSBlurVSMY</PixelEntry>
			</PROGRAM>
			<GrabPass>_ShadowMap</GrabPass>
		</Pass>
		
		<Pass>
			<Tags> <LightMode>ForwardBase</LightMode> </Tags>
			<PROGRAM> 
				<VertexEntry>VS</VertexEntry> 
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
		<Pass>
			<Tags> <LightMode>ForwardAdd</LightMode> </Tags>
			<PROGRAM> 
				<VertexEntry>VS</VertexEntry>
				<PixelEntry>PSAdd</PixelEntry> 
			</PROGRAM>
		</Pass>
		
		<Pass>
			<Tags> <LightMode>PrepassBase</LightMode> </Tags>
			<PROGRAM> 
				<VertexEntry>VSPrepassBase</VertexEntry> 
				<PixelEntry>PSPrepassBase</PixelEntry>
			</PROGRAM>
		</Pass>
		<Pass>
			<Tags> <LightMode>PrepassFinal</LightMode> </Tags>
			<PROGRAM> 
				<VertexEntry>VSPrepassFinal</VertexEntry> 
				<PixelEntry>PSPrepassFinal</PixelEntry>
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>