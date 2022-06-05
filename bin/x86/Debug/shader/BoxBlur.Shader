<Shader Name="BoxBlur">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>tGrpForward</UseTexture>
		<Uniform Name="cbBoxFilter" Slot="3" ShareMode="PerMaterial">
			<Element Name="BoxKernelWeights" Type="float" Count="28" Default="0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111"></Element>
		</Uniform>  

		<Macros> <PASS_ITERATOR_COUNT>3</PASS_ITERATOR_COUNT> </Macros>
		<FileName>BoxBlur</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass Repeat="PASS_ITERATOR_COUNT">
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<Macros>
					<BOX_KERNEL_SIZE>3</BOX_KERNEL_SIZE>
				</Macros>
				<PixelEntry>PS</PixelEntry>
			</PROGRAM>
			<UseGrab>PassOut%^-1:7</UseGrab>
			<GrabPass>PassOut%$+0</GrabPass>
		</Pass>
	</SubShader>
</Shader>