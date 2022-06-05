<Shader Name="Bloom">
	<Include>Standard</Include>
	
	<PROGRAM>
		<UseAttribute>vbSurface</UseAttribute>
		<UseUniform>cbPerFrame</UseUniform>
		<UseUniform>cbPerLight</UseUniform>
		<UseTexture>tGrpPostProc</UseTexture>
		<Uniform Name="cbBloom" Slot="3" ShareMode="PerMaterial">
			<Element Name="_SceneImage_TexelSize" Type="float4"></Element>
			<Element Name="BloomThresholdIntensity" Type="float4" Default="0.25,0.25,0.25,0.8"></Element>
			<Element Name="LensFlareThesholdIntensity" Type="float4" Default="0.1,0.1,0.1,0.2"></Element>
			<Element Name="BlurIterationSpread" Type="float4" Default="2.5,0,0,0"></Element>
			<Element Name="LensColors" Type="float4" Count="4" Default="0.4,0.4,0.8,0.75, 0.4,0.8,0.8,0.75, 0.8,0.4,0.8,0.75, 0.8,0.4,0.0,0.75"></Element>
		</Uniform>

		<Texture>
			<Element Slot="9" Filter="MinMagMipLinear">OrgSceneImage</Element>
		</Texture>
		
		<FileName>Bloom</FileName>
		<VertexEntry>VS</VertexEntry>
	</PROGRAM>
	
	<SubShader>
		<!-- sampling down : 3 pass -->
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
			<UseGrab>DownSample2{bind_slot:8}</UseGrab>
			<GrabPass>DownSample4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSAvgBlur</PixelEntry>
			</PROGRAM>
			<UseGrab>DownSample4{bind_slot:8}</UseGrab>
			<GrabPass>ABlur4{size:0.25}</GrabPass>
		</Pass>
		<!-- threshold : 1 pass -->
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSColorThreshold</PixelEntry>
			</PROGRAM>
			<UseGrab>ABlur4{bind_slot:8}</UseGrab>
			<GrabPass>Threshold4{size:0.25}</GrabPass>
		</Pass>
		<!-- blur : 3 pass -->
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSBlurX</PixelEntry>
			</PROGRAM>
			<UseGrab>Threshold4{bind_slot:8}</UseGrab>
			<GrabPass>GBlurX4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSBlurY</PixelEntry>
			</PROGRAM>
			<UseGrab>GBlurX4{bind_slot:8}</UseGrab>
			<GrabPass>GBlurY4{size:0.25}</GrabPass>
		</Pass>
		<!-- sampling up : 3 pass -->
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSAvgBlur</PixelEntry>
			</PROGRAM>
			<UseGrab>GBlurY4{bind_slot:8}</UseGrab>
			<GrabPass>ABlurY4_1{size:0.5}</GrabPass>
		</Pass>
		<!-- blend : 1 pass -->
		<Pass>
			<Tags> <LightMode>PostProcess1</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSLensThreshold</PixelEntry>
			</PROGRAM>
			<UseGrab>ABlurY4_1{bind_slot:8}</UseGrab>
			<GrabPass>LensThreshold4{size:0.25}</GrabPass>
		</Pass>
		<Pass>
			<Tags> <LightMode>PostProcess</LightMode> </Tags>
			<PROGRAM> 
				<PixelEntry>PSBlendAdd</PixelEntry>
			</PROGRAM>
			<UseGrab>_SceneImage{bind_slot:9}</UseGrab>
			<UseGrab>ABlurY4_1{bind_slot:8}</UseGrab>
		</Pass>
	</SubShader>
</Shader>