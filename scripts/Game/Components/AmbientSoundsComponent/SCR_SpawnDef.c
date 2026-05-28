[BaseContainerProps(configRoot: true)]
class SCR_SpawnDef
{				
	[Attribute("20", UIWidgets.Slider, "", "0 100 1")]
	int m_iSpawnDist;
	
	[Attribute("15", UIWidgets.Slider, "", "0 100 1")]
	int m_iPlayDistMin;
	
	[Attribute("25", UIWidgets.Slider, "", "0 100 1")]
	int m_iPlayDistMax;
	
	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iDensity;
		
	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EWindCurve))]
	EWindCurve m_eWindModifier;
	
	[Attribute(uiwidget: UIWidgets.CurveDialog, desc: "", params:"type=Linear, ends=OpenFlat, default=1, paramRange=0 1 fixed, valueRange=0 1 fixed")]
	ref Curve m_TimeModifier;
		
	[Attribute(uiwidget: UIWidgets.CurveDialog, desc: "", params:"type=Linear, ends=OpenFlat, default=1, paramRange=0 1 fixed, valueRange=0 1 fixed")]
	ref Curve m_RainModifier;
}
