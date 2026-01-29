[BaseContainerProps()]
class SCR_InsectSpawnDef
{
	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iSeaDensityMax;

	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iForestDensityMax;

	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iCityDensityMax;

	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iMeadowDensityMax;

	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iRiverDensityMax;

	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iRiverSlopeDensityMax;

	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iLakeDensityMax;

	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iCoastDensityMax;

	[Attribute("40", UIWidgets.Slider, "", "0 100 1")]
	int m_iDensityConsumption;

	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EWindCurve))]
	EWindCurve m_eWindModifier;

	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "", params: "1 1 0 0")]
	ref Curve m_TimeModifier;

	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "", params: "1 1 0 0")]
	ref Curve m_RainModifier;
}
