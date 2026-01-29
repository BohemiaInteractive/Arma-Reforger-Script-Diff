[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_PlayerUIInfo : SCR_UIInfo
{
	[Attribute()]
	protected int m_iPlayerID;

	//------------------------------------------------------------------------------------------------
	void SetPlayerID(int playerID)
	{
		m_iPlayerID = playerID;
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		return m_iPlayerID;
	}

	//------------------------------------------------------------------------------------------------
	override LocalizedString GetName()
	{
		if (m_iPlayerID > 0)
			return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_iPlayerID);
		else
			return super.GetName();
	}
}
