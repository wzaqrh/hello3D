<Shader Name="ImGui">
	<PROGRAM>
		<Attribute Name="vbImGui">
			<Element SemanticName="POSITION" Format="rg32f"></Element>
			<Element SemanticName="TEXCOORD" Format="rg32f"></Element>
			<Element SemanticName="COLOR" 	 Format="rgba8un"></Element>
		</Attribute>
		<Uniform Name="cbImGui" Slot="3" ShareMode="PerInstance">
			<Element Name="ProjectionMatrix" Type="matrix" Count="0" Offset="0" Default="1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1"></Element>
		</Uniform>
		<Texture>
			<Element Slot="0" Filter="MinMagMipLinear" Address="Wrap">_MainTex</Element>
		</Texture>
		
		<FileName>ImGui</FileName>
		<VertexEntry>VS</VertexEntry>
		
		<Topology1>TriangleList</Topology1>
		<BlendFunc1>Src_A,One_Minus_SrcA</BlendFunc1>
		<FillMode1>Solid</FillMode1>
		<CullMode1>None</CullMode1>
		<DepthEnable1>False</DepthEnable1>
		<DepthFunc1>LessEqual</DepthFunc1>
		<DepthWriteMask1>All</DepthWriteMask1>		
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