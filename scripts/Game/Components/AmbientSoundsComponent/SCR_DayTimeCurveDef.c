const int DAY_TIME_CURVE_COUNT = 7;

enum EDayTimeCurve // TODO: SCR_EDayTimeCurve
{
	ALWAYS,
	DAY,
	NIGHT,
	MORNING,
	NOON,
	AFTERNOON,
	NOTAFTERNOON,
}

//! Used to define a random positional sound behaviour base on time of the day
[BaseContainerProps()]
class SCR_DayTimeCurveDef
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EDayTimeCurve))]
	EDayTimeCurve m_eDayTimeCurve;
	
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "",  params: "1 1 0 0")]
	ref Curve m_Curve;
}
