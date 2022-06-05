<Shader Name="Model">
	<Include>Standard</Include>
	<Include>Model</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseUniform>cbModel</UseUniform>
		<Macros>			
			<ENABLE_TRANSMISSION>1</ENABLE_TRANSMISSION>
			<ENABLE_SHEEN>1</ENABLE_SHEEN>
		</Macros>
		<FileName>Model</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<RenderType>Geometry</RenderType>
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