class PrefabGeneratorPointData : ShapePointDataScriptBase // cannot be renamed SCR_ without changing terrain layers
{
	[Attribute(desc: "Generate assets on this segment")]
	bool m_bGenerate;
}
