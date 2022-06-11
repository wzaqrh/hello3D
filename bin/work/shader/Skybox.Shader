<Shader Name="Skybox">
	<Include>Standard</Include>
	
	<PROGRAM>
		<Attribute>
			<Element SemanticName="POSITION" Format="rgb32f"></Element>
		</Attribute>
		<UseUniform>cbPerFrame</UseUniform>
		<Texture>
			<Element Slot="0" Filter="MinMagMipLinear" Address="Clamp">_MainTex</Element>
		</Texture>
		
		<FileName>Skybox</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<RenderType>Background</RenderType>
		<Pass>
			<LightMode>ForwardBase</LightMode>
			<PROGRAM> <PixelEntry>PS</PixelEntry> </PROGRAM>
		</Pass>
	</SubShader>
</Shader>