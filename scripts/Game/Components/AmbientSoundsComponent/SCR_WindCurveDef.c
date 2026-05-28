// TODO: store it in a class instead of a GLOBAL variable
const int WIND_CURVE_COUNT = 3;

enum EWindCurve
{
	ALWAYS,
	DEFAULT,
	CREAKING,
}

[BaseContainerProps()]
class SCR_WindCurveDef
{	
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EWindCurve))]
	EWindCurve m_eWindCurveType;
	
	[Attribute(uiwidget: UIWidgets.CurveDialog, desc: "", params:"type=Linear, ends=OpenFlat, default=1, paramRange=0 1 fixed, valueRange=0 1 fixed")]
	ref Curve m_Curve;
}
