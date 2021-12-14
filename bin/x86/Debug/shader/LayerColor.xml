<Shader Name="LayerColor">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>txGlobal</UseTexture>
		
		<FileName>LayerColor</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<Tags> <LightMode>ShadowCaster</LightMode> </Tags>
			<PROGRAM> 
				<VertexEntry>VSShadowCaster</VertexEntry> 
				<PixelEntry></PixelEntry>
			</PROGRAM>
		</Pass>
		<Pass>
			<Tags> <LightMode>ForwardBase</LightMode> </Tags>
			<PROGRAM>
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>