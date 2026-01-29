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
	
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "",  params: "1 1 0 0")]
	ref Curve m_Curve;
}
