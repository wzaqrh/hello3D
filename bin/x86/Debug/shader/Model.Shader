<Shader Name="Model">
	<Include>Standard</Include>
	<Include>Skeleton</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseAttribute>vbWeightedSkin</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseUniform>cbWeightedSkin</UseUniform>
		<Uniform Name="cbModel" Slot="3" ShareMode="PerMaterial">
			<Element Name="AlbedoTransUV"			Type="float4" Default="0,0,1,1"></Element>
			<Element Name="NormalTransUV"			Type="float4" Default="0,0,1,1"></Element>
			<Element Name="AoTransUV"				Type="float4" Default="0,0,1,1"></Element>
			<Element Name="RoughnessTransUV"        Type="float4" Default="0,0,1,1"></Element>
			<Element Name="MetallicTransUV"         Type="float4" Default="0,0,1,1"></Element>
			<Element Name="EmissiveTransUV"         Type="float4" Default="0,0,1,1"></Element>
			
			<Element Name="AlbedoFactor"            Type="float4" Default="1,1,1,1"></Element>
			<Element Name="EmissiveFactor"          Type="float4" Default="1,1,1,1"></Element>
			<Element Name="NormalScale"             Type="float"  Default="1"></Element>
			<Element Name="AoStrength"				Type="float"  Default="1"></Element>
			<Element Name="RoughnessFactor"         Type="float"  Default="1"></Element>
			<Element Name="MetallicFactor"          Type="float"  Default="1"></Element>
			<Element Name="TransmissionFactor"      Type="float"  Default="0"></Element>		
		</Uniform>   
		<Texture>
			<Element Slot="0" Address="Wrap">txAlbedo</Element>
			<Element Slot="1" Address="Wrap">txNormal</Element>
			<Element Slot="2" Address="Wrap">txMetallic</Element>
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
		<DepthEnable1>True</DepthEnable1>
		<DepthFunc1>LessEqual</DepthFunc1>
		<DepthWriteMask1>All</DepthWriteMask1>
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
			<UseGrab>_ShadowMap{bind_slot:9}</UseGrab>
			<LightMode>ShadowCaster_PostProcess</LightMode>
			<PROGRAM>
				<UseTexture>tGrpPostProc</UseTexture>
				<Texture>
					<Element Slot="9" Filter="MinMagMipLinear">PrePassOutput</Element>
				</Texture>
				<VertexEntry>VSBlurVSM</VertexEntry>
				<PixelEntry>PSBlurVSMX</PixelEntry>
			</PROGRAM>
			<GrabPass>VSM_BlurX{fmts:[16,45]}</GrabPass>
		</Pass>
		<Pass>
			<UseGrab>VSM_BlurX{bind_slot:9}</UseGrab>
			<LightMode>ShadowCaster_PostProcess</LightMode>
			<PROGRAM>
				<UseTexture>tGrpPostProc</UseTexture>
				<Texture>
					<Element Slot="9" Filter="MinMagMipLinear">PrePassOutput</Element>
				</Texture>
				<VertexEntry>VSBlurVSM</VertexEntry>
				<PixelEntry>PSBlurVSMY</PixelEntry>
			</PROGRAM>
			<GrabPass>_ShadowMap</GrabPass>
		</Pass>
		
		<Pass>
			<UseGrab Condition="ENABLE_TRANSMISSION==1">_geometry_skybox{bind_slot:8}</UseGrab>
			<LightMode>ForwardBase</LightMode>
			<PROGRAM>
				<UseTexture>tGrpForward</UseTexture>
				<VertexEntry>VS</VertexEntry> 
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
		<Pass>
			<LightMode>ForwardAdd</LightMode>
			<PROGRAM> 
				<UseTexture>tGrpForward</UseTexture>
				<VertexEntry>VS</VertexEntry>
				<PixelEntry>PSAdd</PixelEntry> 
			</PROGRAM>
		</Pass>
		
		<Pass>
			<LightMode>PrepassBase</LightMode>
			<PROGRAM> 
				<UseTexture>tGrpForward</UseTexture>
				<VertexEntry>VSPrepassBase</VertexEntry> 
				<PixelEntry>PSPrepassBase</PixelEntry>
			</PROGRAM>
		</Pass>
		<Pass>
			<UseGrab Condition="ENABLE_TRANSMISSION==1">_geometry_skybox{bind_slot:8}</UseGrab>
			<LightMode>PrepassFinal</LightMode>
			<PROGRAM> 
				<UseTexture>tGrpPrepassFinal</UseTexture>
				<VertexEntry>VSPrepassFinal</VertexEntry> 
				<PixelEntry>PSPrepassFinal</PixelEntry>
			</PROGRAM>
		</Pass>
		<Pass>
			<LightMode>PrepassFinalAdd</LightMode>
			<PROGRAM> 
				<UseTexture>tGrpPrepassFinal</UseTexture>
				<VertexEntry>VSPrepassFinal</VertexEntry> 
				<PixelEntry>PSPrepassFinalAdd</PixelEntry>
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>