enum ETerrainType
{
	MEADOW_GRASS_SHORT,
	MEADOW_GRASS_TALL,
	FOREST_CONIFER,
	FOREST_LEAFY,
	CROPFIELD,
	CONCRETE,
	OCEAN,
	POND,
	POND_SMALL
}

[BaseContainerProps()]
class SCR_TerrainDef
{
	[Attribute("0", UIWidgets.EditBox, "")]
	int m_iRangeMin;
	
	[Attribute("0", UIWidgets.EditBox, "")]
	int m_iRangeMax;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ETerrainType))]
	ETerrainType m_eTerrainType;	
}
