<Shader Name="LayerColor">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>tGrpForward</UseTexture>
		
		<Topology Condition="POINT_MODE==1">1</Topology>
		<Topology Condition="LINE_MODE==1">2</Topology>
		<FileName>LayerColor</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<RenderType>Overlay</RenderType>
		<Pass>
			<LightMode>ForwardBase</LightMode>
			<PROGRAM>
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>