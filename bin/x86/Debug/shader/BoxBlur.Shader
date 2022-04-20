<Shader Name="BoxBlur">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>txGlobal</UseTexture>
		<Uniform Name="cbBoxFilter" Slot="3" ShareMode="1">
			<Element Name="BoxKernelWeights" Type="float" Count="28" Default="0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111"></Element>
		</Uniform>  

		<FileName>BoxBlur</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<Macros>
					<BOX_KERNEL_SIZE>3</BOX_KERNEL_SIZE>
				</Macros>
				<PixelEntry>PS</PixelEntry> 
			</PROGRAM>
		</Pass>
	</SubShader>
</Shader>