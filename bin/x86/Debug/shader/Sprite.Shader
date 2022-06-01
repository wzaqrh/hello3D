<Shader Name="Sprite">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>txGlobal</UseTexture>
		<Texture>
			<Element Slot="0">_MainTex</Element>
		</Texture>
		
		<FileName>Sprite</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<RenderType>Overlay</RenderType>
			<LightMode>ForwardBase</LightMode>
			<PROGRAM> 
				<Macros>
					<DEBUG_SHADOW_MAP>0</DEBUG_SHADOW_MAP>
				</Macros>
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>