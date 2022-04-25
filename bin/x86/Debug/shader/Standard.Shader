<Shader Name="Standard">
	<PROGRAM>		
		<Attribute Name="vbSurface">
			<Element SemanticName="POSITION" Format="6" SemanticIndex="0"></Element>
			<Element SemanticName="COLOR" 	 Format="28"></Element>
			<Element SemanticName="TEXCOORD" Format="16"></Element>
		</Attribute>
		
		<Uniform Name="cbPerFrame" Slot="0" ShareMode="2">
			<Element Name="World" 		  	Type="matrix" Count="0" Offset="0" Default="1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1"></Element>
			<Element Name="View" 	 	    Type="matrix"></Element>
			<Element Name="Projection" 		Type="matrix"></Element>
			
			<Element Name="ViewInv" 	    Type="matrix"></Element>
			<Element Name="ProjectionInv"	Type="matrix"></Element>
			
			<Element Name="LightView" 	 	Type="matrix"></Element>
			<Element Name="LightProjection" Type="matrix"></Element>

			<Element Name="CameraPosition"  Type="float4"></Element>
			<Element Name="FrameBufferSize" Type="float4"></Element>
			<Element Name="ShadowMapSize"   Type="float4"></Element>
			<Element Name="glstate_lightmodel_ambient"  Type="float4" Default="0.1,0.1,0.1,0.1"></Element>
			<Element Name="Exposure"        Type="float"></Element>
		</Uniform>
		<Uniform Name="cbPerLight" Slot="1" ShareMode="2">
			<Element Name="unity_LightPosition"	Type="float4"></Element>
			<Element Name="unity_LightColor"	Type="float4"></Element> 
			<Element Name="unity_SpecColor"		Type="float4"></Element>
			<Element Name="unity_LightAtten"	Type="float4"></Element>
			<Element Name="unity_SpotDirection"	Type="float4"></Element>
			<Element Name="LightRadiusUVNearFar" Type="float4"></Element>
			<Element Name="LightDepthParam"		Type="float4"></Element>
			<Element Name="IsSpotLight" 	 	Type="bool"></Element>
		</Uniform>
		
		<Texture Name="txDepth">
			<Element Slot="7" Filter="0x15">_GDepth</Element>
		</Texture>
		<Texture Name="txShadowMap">
			<Element Slot="8" CompFunc="2">_ShadowMapTexture</Element>
		</Texture>
		<Texture Name="txEnv">
			<Element Slot="9" Filter="0x15" AddressU="3" AddressV="3" AddressW="1">_DiffuseCube</Element>
			<Element Slot="10" Filter="0x15" AddressU="3" AddressV="3" AddressW="1">_SpecCube</Element>
			<Element Slot="11" Filter="0x15">_LUT</Element>			
		</Texture>
		<Texture Name="txGBuffer">
			<Element Slot="12" Filter="0x15">_GBufferPos</Element>
			<Element Slot="13" Filter="0x15">_GBufferNormal</Element>
			<Element Slot="14" Filter="0x15">_GBufferAlbedo</Element>
		    <Element Slot="15" Filter="0x15">_GBufferEmissive</Element>
		</Texture>
		
		<Texture Name="txGlobal">
			<Element Slot="6" Filter="0x15">_SceneImage</Element>
			<Element Slot="7" Filter="0x0">_GDepth</Element>
			<Element Slot="8" CompFunc="2" AddressU="4" AddressV="4" AddressW="1">_ShadowMapTexture</Element>
			<Element Slot="9" Filter="0x15" AddressU="3" AddressV="3" AddressW="1">_DiffuseCube</Element>
			<Element Slot="10" Filter="0x15" AddressU="3" AddressV="3" AddressW="1">_SpecCube</Element>
			<Element Slot="11" Filter="0x15">_LUT</Element>			
			<Element Slot="12" Filter="0x15">_GBufferPos</Element>
			<Element Slot="13" Filter="0x15">_GBufferNormal</Element>
			<Element Slot="14" Filter="0x15">_GBufferAlbedo</Element>
		    <Element Slot="15" Filter="0x15">_GBufferEmissive</Element>
		</Texture>
	</PROGRAM>
</Shader>