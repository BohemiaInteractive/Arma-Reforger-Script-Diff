[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_BloodTypeUiInfo")]
class SCR_CharacterIdentityBloodType
{
	[Attribute("-1", desc: "Array of blood types is defined in Extended Identity manager. NEVER SET TO AUTO in this array!", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EBloodType))]
	protected SCR_EBloodType m_eBloodType;
	
	[Attribute(desc: "UI info for bloodtype")]
	protected ref SCR_UIInfo m_BloodTypeUiInfo;
	
	[Attribute(desc: "Weight of blood type. The higher the weight the more chance it is picked in randomization.")]
	protected int m_iBloodTypeWeight;
	
	//------------------------------------------------------------------------------------------------
	SCR_EBloodType GetBloodType()
	{
		return m_eBloodType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetUiInfo()
	{
		return m_BloodTypeUiInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetWeight()
	{
		return m_iBloodTypeWeight;
	}
}