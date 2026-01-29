[BaseContainerProps(configRoot:true)]
class SCR_ArsenalItemTypeUIConfig
{
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_ArsenalItemTypeUI> m_ArsenalItemUITypes;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_ArsenalItemTypeUI> GetAvailableItems()
	{
		return m_ArsenalItemUITypes;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] arsenalItemType
	//! \param[out] uiInfo
	//! \return
	bool GetInfoForType(SCR_EArsenalItemType arsenalItemType, out SCR_UIInfo uiInfo)
	{
		for (int i = m_ArsenalItemUITypes.Count() - 1; i >= 0; i--)
		{
			SCR_ArsenalItemTypeUI itemTypeUI = m_ArsenalItemUITypes[i];
			if (itemTypeUI && itemTypeUI.GetItemType() == arsenalItemType)
			{
				uiInfo = itemTypeUI.GetInfo();
				return true;
			}
		}
		return false;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EArsenalItemType, "m_eItemType")]
class SCR_ArsenalItemTypeUI
{	
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eItemType;
	
	[Attribute("", UIWidgets.Object)]
	protected ref SCR_UIInfo m_Info;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemType GetItemType()
	{
		return m_eItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
}
