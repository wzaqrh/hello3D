<Shader Name="Skeleton">
	<PROGRAM>
		<Attribute Name="vbWeightedSkin">
			<Element Format="6"  SemanticName="NORMAL"></Element>
			<Element Format="6"  SemanticName="NORMAL" SemanticIndex="1"></Element>
			<Element Format="6"  SemanticName="NORMAL" SemanticIndex="2"></Element>
			<Element Format="2"  SemanticName="BLENDWEIGHT"></Element>
			<Element Format="3"  SemanticName="BLENDINDICES"></Element>
		</Attribute>
		<Uniform Name="cbWeightedSkin" Slot="2">
			<Element Name="Model" Type="matrix"></Element>
			<Element Name="Models" Type="matrix" Count="56"></Element>
		</Uniform>
	</PROGRAM>
</Shader>