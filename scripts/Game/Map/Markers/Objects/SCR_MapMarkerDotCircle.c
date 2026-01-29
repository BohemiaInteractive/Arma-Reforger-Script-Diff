class SCR_MapMarkerDotCircleClass : SCR_MapMarkerEntityClass
{
}

class SCR_MapMarkerDotCircle : SCR_MapMarkerEntity
{
	[Attribute(defvalue: "1000", desc: "Radius", params: "0 inf 0.01")]
	float m_fRadius;

	[Attribute(defvalue: "1 0 0 1", desc: "Dot Color")]
	ref Color m_DotColor;

	[Attribute(defvalue: "0.02", desc: "Dot Density", params: "0 inf 0.01")]
	float m_fDotDensity;
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		SetGlobalVisible(true);
		m_eType = SCR_EMapMarkerType.DOT_CIRCLE;
		m_Target = this;
	}
	
}