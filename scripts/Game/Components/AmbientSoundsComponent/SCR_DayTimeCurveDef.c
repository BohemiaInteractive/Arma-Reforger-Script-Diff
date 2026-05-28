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
	
	[Attribute(uiwidget: UIWidgets.CurveDialog, desc: "", params:"type=Linear, ends=OpenFlat, default=1, paramRange=0 1 fixed, valueRange=0 1 fixed")]
	ref Curve m_Curve;
}
