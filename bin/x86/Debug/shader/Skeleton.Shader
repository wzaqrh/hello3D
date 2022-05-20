<Shader Name="Skeleton">
	<PROGRAM>
		<Attribute Name="vbWeightedSkin">
			<Element Format="rgb32f"  SemanticName="NORMAL"></Element>
			<Element Format="rgba32f" SemanticName="NORMAL" SemanticIndex="1"></Element>
			<Element Format="rgb32f"  SemanticName="NORMAL" SemanticIndex="2"></Element>
			<Element Format="rgba32f" SemanticName="BLENDWEIGHT"></Element>
			<Element Format="rgba32u" SemanticName="BLENDINDICES"></Element>
		</Attribute>
		<Uniform Name="cbWeightedSkin" Slot="2">
			<Element Name="Model" Type="matrix" Default="1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1"></Element>
			<Element Name="Models" Type="matrix" Count="56"></Element>
		</Uniform>
	</PROGRAM>
</Shader>