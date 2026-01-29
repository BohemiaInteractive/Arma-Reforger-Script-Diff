[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "ResupplyData", "DISABLED - ResupplyData", 1)]
class SCR_EntityCatalogSupportStationResupplyData: SCR_BaseEntityCatalogData
{
	[Attribute(desc: "Resupply type used by Resupply support station. The first entry with the type will be used for the resupply action", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ESupportStationResupplyType))]
	protected SCR_ESupportStationResupplyType m_eResupplyType; 
	
	//------------------------------------------------------------------------------------------------
	SCR_ESupportStationResupplyType GetResupplyType()
	{
		return m_eResupplyType;
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_ESupportStationResupplyType
{
	NONE,
	GRENADE,
	SMOKE,
	BANDAGE,
	MORPHINE,
	MORTAR_SHELL,
}
