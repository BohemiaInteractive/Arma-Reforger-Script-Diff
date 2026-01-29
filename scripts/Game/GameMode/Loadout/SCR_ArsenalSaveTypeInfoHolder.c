[BaseContainerProps(configRoot: true)]
class SCR_ArsenalSaveTypeInfoHolder
{
	[Attribute(desc: "List of UI info for each arsenal save types UI Info")]
	protected ref array<ref SCR_ArsenalSaveTypeInfo> m_aArsenalSaveTypeUIInfoList;
	
	//------------------------------------------------------------------------------------------------
	//! Get List of all save type Info
	//! \param[out] arsenalSaveTypeUIInfoList List of save types
	//! \return Count of save types
	int GetArsenalSaveTypeInfoList(out notnull array<SCR_ArsenalSaveTypeInfo> arsenalSaveTypeUIInfoList)
	{
		arsenalSaveTypeUIInfoList.Clear();
		
		foreach (SCR_ArsenalSaveTypeInfo saveType : m_aArsenalSaveTypeUIInfoList)
		{
			if (!saveType)
				continue;
			
			arsenalSaveTypeUIInfoList.Insert(saveType);
		}
		
		return arsenalSaveTypeUIInfoList.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get UIInfo of given Type
	//! \param[in] saveTypeToCheck Save type to get UI info of
	//! \return type's UI info
	SCR_ArsenalSaveTypeUIInfo GetUIInfoOfType(SCR_EArsenalSaveType saveTypeToCheck)
	{
		foreach (SCR_ArsenalSaveTypeInfo saveType : m_aArsenalSaveTypeUIInfoList)
		{
			if (!saveType)
				continue;
			
			if (saveType.GetSaveType() == saveTypeToCheck)
				return saveType.GetUIInfo();
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if given Save type is in the list
	//! \param[in] saveTypeToCheck Save type to check
	//! \return True if save type is in the list
	bool HasSaveType(SCR_EArsenalSaveType saveTypeToCheck)
	{
		foreach (SCR_ArsenalSaveTypeInfo saveType : m_aArsenalSaveTypeUIInfoList)
		{
			if (!saveType)
				continue;
			
			if (saveType.GetSaveType() == saveTypeToCheck)
				return true;
		}
		
		return false;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EArsenalSaveType, "m_eArsenalSaveType")]
class SCR_ArsenalSaveTypeInfo
{
	[Attribute("Save type associated with the with the UIInfo", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalSaveType))]
	protected SCR_EArsenalSaveType m_eArsenalSaveType;
	
	[Attribute("UI info of arsenal save type. To display the display name and description")]
	protected ref SCR_ArsenalSaveTypeUIInfo m_UIInfo;
	
	//------------------------------------------------------------------------------------------------
	//! \return Get Save type
	SCR_EArsenalSaveType GetSaveType()
	{
		return m_eArsenalSaveType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get UIInfo
	SCR_ArsenalSaveTypeUIInfo GetUIInfo()
	{
		return m_UIInfo;
	}
}
