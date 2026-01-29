//! Array has to have the same size as EWindCurve count
[BaseContainerProps(configRoot: true)]
class SCR_WindCurveDefConfig
{
	[Attribute(desc: "Wind modifier curve definition")]
	ref array<ref SCR_WindCurveDef> m_aWindModifier;
}
