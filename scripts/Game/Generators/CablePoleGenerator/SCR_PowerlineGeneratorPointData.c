class SCR_PowerlineGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute(defvalue: "0", desc: "Offset of the poles after this point, in metres")]
	float m_fPoleOffset;

	[Attribute(defvalue: "0", desc: "Generate poles on anchor points until a point with this attribute unchecked is reached.")]
	bool m_bGeneratePerPoint;

	[Attribute()]
	ref SCR_PowerlineGeneratorJunctionData m_JunctionData;
}
