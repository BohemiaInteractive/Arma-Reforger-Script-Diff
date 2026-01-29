[BaseContainerProps(configRoot: true)]
class SCR_GeneralHintStorage
{
	[Attribute()]
	protected ref array<ref SCR_HintEntry> m_aHintEntries;

	//------------------------------------------------------------------------------------------------
	SCR_HintUIInfo GetHintByType(EHint type)
	{
		foreach (SCR_HintEntry hintEntry : m_aHintEntries)
		{
			if (hintEntry.GetHintType() == type)
				return hintEntry.GetHintInfo();
		}
		
		return null;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EHint, "m_HintType")]
class SCR_HintEntry
{
	[Attribute()]
	protected ref SCR_HintUIInfo m_HintInfo;
	
	//------------------------------------------------------------------------------------------------
	EHint GetHintType()
	{
		return m_HintInfo.GetType();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_HintUIInfo GetHintInfo()
	{
		return m_HintInfo;
	}
}