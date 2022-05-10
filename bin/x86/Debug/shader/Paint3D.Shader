<Shader Name="LayerColor">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>txGlobal</UseTexture>
		
		<Topology Condition="POINT_MODE==1">1</Topology>
		<Topology Condition="LINE_MODE==1">2</Topology>
		<FileName>LayerColor</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<Tags> <LightMode>Transparent</LightMode> </Tags>
			<PROGRAM>
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>