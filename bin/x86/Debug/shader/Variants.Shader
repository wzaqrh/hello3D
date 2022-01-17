<ShaderVariants>
	<Variant Name="Cube">
		<AliasName>test_shadow_map_case2</AliasName>
		<UseShader>LayerColor</UseShader>
		<PROGRAM>
			<Macros>
				<ENABLE_SHADOW_MAP>1</ENABLE_SHADOW_MAP>
				<DEBUG_SHADOW_MAP>0</DEBUG_SHADOW_MAP>
			</Macros>
		</PROGRAM>
	</Variant>
	
	<Variant Name="test_shadow_map_case3">
		<UseShader>LayerColor</UseShader>
		<PROGRAM>
			<Macros>
				<ENABLE_SHADOW_MAP>1</ENABLE_SHADOW_MAP>
				<DEBUG_SHADOW_MAP>1</DEBUG_SHADOW_MAP>
			</Macros>
		</PROGRAM>
	</Variant>
	
	<Variant Name="Skybox/Deprecate">
		<UseShader>Skybox</UseShader>
		<PROGRAM>
			<Topology>5</Topology>
			<Macros>
				<DEPRECATE_SKYBOX>1</DEPRECATE_SKYBOX>
			</Macros>
		</PROGRAM>
	</Variant>
</ShaderVariants>