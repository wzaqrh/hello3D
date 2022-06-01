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
		<Uniform Name="cbModel" Slot="3" ShareMode="PerMaterial">
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
			<Element Name="TransmissionFactor"      Type="float"  Default="0"></Element>
			<Element Name="EmissiveFactor"          Type="float4" Default="1,1,1,1"></Element>
		</Uniform>   
		<Texture>
			<Element Slot="0" Address="Wrap">txAlbedo</Element>
			<Element Slot="1" Address="Wrap">txNormal</Element>
			<Element Slot="2" Address="Wrap">txMetalness</Element>
			<Element Slot="3" Address="Wrap">txRoughness</Element>
			<Element Slot="4" Address="Wrap">txAmbientOcclusion</Element>
			<Element Slot="5" Address="Wrap">txEmissive</Element>
		</Texture>
		<Macros>			
			<ENABLE_ALBEDO_MAP>0</ENABLE_ALBEDO_MAP>
			<ALBEDO_MAP_SRGB>1</ALBEDO_MAP_SRGB>
			
			<ENABLE_EMISSIVE_MAP>0</ENABLE_EMISSIVE_MAP>
			<EMISSIVE_MAP_SRGB>1</EMISSIVE_MAP_SRGB>
			
			<ENABLE_AO_ROUGHNESS_METALLIC_MAP>0</ENABLE_AO_ROUGHNESS_METALLIC_MAP>
			<ENABLE_METALLIC_X_X_SMOOTHNESS_MAP>0</ENABLE_METALLIC_X_X_SMOOTHNESS_MAP>
			<ENABLE_AO_MAP>0</ENABLE_AO_MAP>
			<ENABLE_ROUGHNESS_MAP>0</ENABLE_ROUGHNESS_MAP>
			<ENABLE_METALLIC_MAP>0</ENABLE_METALLIC_MAP>
			
			<HAS_ATTRIBUTE_NORMAL>1</HAS_ATTRIBUTE_NORMAL>
			<HAS_ATTRIBUTE_TANGENT>1</HAS_ATTRIBUTE_TANGENT>
			<ENABLE_PIXEL_BTN>1</ENABLE_PIXEL_BTN>
		</Macros>
		<FileName>Model</FileName>
		<VertexEntry>VS</VertexEntry>
		
		<FillMode>Solid</FillMode>
		<CullMode>Back</CullMode>
		<DepthEnable>True</DepthEnable>
		<DepthFunc>LessEqual</DepthFunc>
		<DepthWriteMask>All</DepthWriteMask>
	</PROGRAM>
	
	<SubShader>
		<RenderType>Geometry</RenderType>
		<Pass>
			<LightMode>ShadowCaster</LightMode>
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
			<LightMode>ShadowCaster+1</LightMode>
			<PROGRAM>
				<VertexEntry>VSBlurVSM</VertexEntry>
				<PixelEntry>PSBlurVSMX</PixelEntry>
			</PROGRAM>
			<GrabPass>VSM_BlurX{fmts:[16,45]}</GrabPass>
		</Pass>
		<Pass>
			<UseGrab>VSM_BlurX{bind_slot:14}</UseGrab>
			<LightMode>ShadowCaster+1</LightMode>
			<PROGRAM>
				<VertexEntry>VSBlurVSM</VertexEntry>
				<PixelEntry>PSBlurVSMY</PixelEntry>
			</PROGRAM>
			<GrabPass>_ShadowMap</GrabPass>
		</Pass>
		
		<Pass>
			<LightMode>ForwardBase</LightMode>
			<PROGRAM> 
				<VertexEntry>VS</VertexEntry> 
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
		<Pass>
			<LightMode>ForwardAdd</LightMode>
			<PROGRAM> 
				<VertexEntry>VS</VertexEntry>
				<PixelEntry>PSAdd</PixelEntry> 
			</PROGRAM>
		</Pass>
		
		<Pass>
			<LightMode>PrepassBase</LightMode>
			<PROGRAM> 
				<VertexEntry>VSPrepassBase</VertexEntry> 
				<PixelEntry>PSPrepassBase</PixelEntry>
			</PROGRAM>
		</Pass>
		<Pass>
			<LightMode>PrepassFinal</LightMode>
			<PROGRAM> 
				<VertexEntry>VSPrepassFinal</VertexEntry> 
				<PixelEntry>PSPrepassFinal</PixelEntry>
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>