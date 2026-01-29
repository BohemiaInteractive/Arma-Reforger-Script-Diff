[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EGameOverTypes, "m_GameOverScreenId")]
class SCR_RandomMessageGameOverScreenInfo : SCR_BaseGameOverScreenInfo
{
	[Attribute(desc: "Randomized message shown in the end screen. Will use value set in Optional Params if this array is empty")]
	protected ref array<LocalizedString> m_aRandomEndMessages;
	
	//------------------------------------------------------------------------------------------------
	override string GetSubtitle(Faction factionPlayer, array<Faction> factionsOther)
	{
		if (m_aRandomEndMessages.IsEmpty())
			return super.GetSubtitle(factionPlayer, factionsOther);
		
		return m_aRandomEndMessages.GetRandomElement();
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetSubtitle(int playerId, array<int> otherPlayerIds)
	{
		if (m_aRandomEndMessages.IsEmpty())
			return super.GetSubtitle(playerId, otherPlayerIds);
		
		return m_aRandomEndMessages.GetRandomElement();
	}
}
