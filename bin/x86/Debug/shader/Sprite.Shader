<Shader Name="Sprite">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>tGrpForward</UseTexture>
		<Texture>
			<Element Slot="0">_MainTex</Element>
		</Texture>
		
		<FileName>Sprite</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<RenderType>Overlay</RenderType>
		<Pass>
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