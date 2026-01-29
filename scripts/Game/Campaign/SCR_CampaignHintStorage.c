//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_CampaignHintStorage
{
	[Attribute()]
	protected ref array<ref SCR_CampaignHintEntry> m_aHintEntries;

	//------------------------------------------------------------------------------------------------
	SCR_HintUIInfo GetHintByEnum(EHint id)
	{
		foreach (SCR_CampaignHintEntry hintEntry : m_aHintEntries)
		{
			if (hintEntry.GetHintId() == id)
				return hintEntry.GetHintInfo();
		}
		
		return null;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EHint, "m_eHintId")]
class SCR_CampaignHintEntry
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EHint))]
	protected EHint m_eHintId;

	[Attribute()]
	protected ref SCR_HintUIInfo m_HintInfo;
	
	//------------------------------------------------------------------------------------------------
	EHint GetHintId()
	{
		return m_eHintId;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_HintUIInfo GetHintInfo()
	{
		return m_HintInfo;
	}
}