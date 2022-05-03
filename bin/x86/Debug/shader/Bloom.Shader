<Shader Name="Bloom">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>txGlobal</UseTexture>
		<Uniform Name="cbBoxFilter" Slot="3" ShareMode="1">
			<Element Name="BoxKernelWeights" Type="float" Count="28" Default="0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111,0.11111"></Element>
		</Uniform>  

		<FileName>Bloom</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSDownSample</PixelEntry>
			</PROGRAM>
			<GrabPass>DownSample2{size:0.5}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSDownSample</PixelEntry>
			</PROGRAM>
			<UseGrab>DownSample2{bind_slot:6}</UseGrab>
			<GrabPass>DownSample4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSAvgBlur</PixelEntry>
			</PROGRAM>
			<UseGrab>DownSample4{bind_slot:6}</UseGrab>
			<GrabPass>ABlur4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSColorThreshold</PixelEntry>
			</PROGRAM>
			<UseGrab>ABlur4{bind_slot:6}</UseGrab>
			<GrabPass>Threshold4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSBlurX</PixelEntry>
			</PROGRAM>
			<UseGrab>Threshold4{bind_slot:6}</UseGrab>
			<GrabPass>GBlurX4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSBlurY</PixelEntry>
			</PROGRAM>
			<UseGrab>GBlurX4{bind_slot:6}</UseGrab>
			<GrabPass>GBlurY4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSAvgBlur</PixelEntry>
			</PROGRAM>
			<UseGrab>GBlurY4{bind_slot:6}</UseGrab>
			<GrabPass>ABlurY4_1{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSLensThreshold</PixelEntry>
			</PROGRAM>
			<UseGrab>ABlurY4_1{bind_slot:6}</UseGrab>
			<GrabPass>LensThreshold4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSBlendAdd</PixelEntry>
			</PROGRAM>
			<UseGrab>__process_input{bind_slot:14}</UseGrab>
			<UseGrab>LensThreshold4{bind_slot:6}</UseGrab>
		</Pass>
	</SubShader>
</Shader>