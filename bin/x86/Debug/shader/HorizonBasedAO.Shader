<Shader Name="HorizonBasedAO">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>txGlobal</UseTexture>
		<Uniform Name="cbHBAo" Slot="3" ShareMode="1">
			<Element Name="DepthParam" Type="float4" Default="3.33333,3.33233"></Element> 
			<Element Name="FocalLen" Type="float4" Default="1.0,1.0,1.0,1.0"></Element>
			<Element Name="Radius" Type="float4" Default="0.35,0.1225,2.857"></Element>
			<Element Name="NumStepDirContrast" Type="float4" Default="8,16,1.25"></Element>
			<Element Name="AttenTanBias" Type="float4" Default="1.0,30"></Element>
		</Uniform>
		<FileName>HorizonBasedAO</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<Macros>
				</Macros>
				<PixelEntry>PS</PixelEntry>
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>