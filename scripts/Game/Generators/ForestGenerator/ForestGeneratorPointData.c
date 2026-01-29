class ForestGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute("1", desc: "Generate the middle outline along this line")]
	bool m_bMiddleOutline;

	[Attribute("1", desc: "Generate the small outline along this line")]
	bool m_bSmallOutline;
}
