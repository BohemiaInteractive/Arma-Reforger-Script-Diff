//! Array size has to have the same count as EDayTimeCurve
[BaseContainerProps(configRoot: true)]
class SCR_DayTimeCurveDefConfig
{
	[Attribute("", UIWidgets.Object, "Day time factor curve definition")]
	ref array<ref SCR_DayTimeCurveDef> m_aDayTimeCurve;
}
