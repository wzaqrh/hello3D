<Shader Name="Skybox">
	<Include>Standard</Include>
	
	<PROGRAM>
		<Attribute>
			<Element SemanticName="POSITION" Format="6"></Element>
		</Attribute>
		<UseUniform>cbPerFrame</UseUniform>
		<Texture>
			<Element Slot="0" Filter="0x15" Address="3">_MainTex</Element>
		</Texture>
		
		<FileName>Skybox</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<Tags> <LightMode>Skybox</LightMode> </Tags>
			<PROGRAM> <PixelEntry>PS</PixelEntry> </PROGRAM>
		</Pass>
	</SubShader>
</Shader>