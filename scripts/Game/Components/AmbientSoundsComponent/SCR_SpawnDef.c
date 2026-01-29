enum EEnvironmentType
{
	MEADOW,
	FOREST,
	HOUSES,
	SEA
}

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
	int m_iMeadowDensityMax;
	
	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iForestDensityMax;
	
	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iHousesDensityMax;
	
	[Attribute("100", UIWidgets.Slider, "", "0 500 1")]
	int m_iSeaDensityMax;
	
	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EWindCurve))]
	EWindCurve m_eWindModifier;
	
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "",  params: "1 1 0 0")]
	ref Curve m_TimeModifier;
		
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "",  params: "1 1 0 0")]
	ref Curve m_RainModifier;
}
