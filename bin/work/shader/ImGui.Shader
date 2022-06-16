<Shader Name="ImGui">
	<Include>Standard</Include>
	
	<PROGRAM>
		<Attribute Name="vbImGui">
			<Element SemanticName="POSITION" Format="rg32f"></Element>
			<Element SemanticName="TEXCOORD" Format="rg32f"></Element>
			<Element SemanticName="COLOR" 	 Format="rgba8un"></Element>
		</Attribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<Uniform Name="cbImGui" Slot="3" ShareMode="PerInstance">
			<Element Name="ProjectionMatrix" Type="matrix" Count="0" Offset="0" Default="1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1"></Element>
		</Uniform>
		<Texture>
			<Element Slot="0" Filter="MinMagMipLinear" Address="Wrap">_MainTex</Element>
		</Texture>
		
		<FileName>ImGui</FileName>
		<VertexEntry>VS</VertexEntry>
		
		<Topology>TriangleList</Topology>
		<BlendFunc>Src_A,One_Minus_SrcA</BlendFunc>
		<FillMode>Solid</FillMode>
		<CullMode>None</CullMode>
		<DepthEnable>False</DepthEnable>
		<DepthFunc>LessEqual</DepthFunc>
		<DepthWriteMask>All</DepthWriteMask>		
	</PROGRAM>
	
	<SubShader>
		<RenderType>UI</RenderType>
		<Pass>
			<LightMode>ForwardBase</LightMode>
			<PROGRAM>
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>