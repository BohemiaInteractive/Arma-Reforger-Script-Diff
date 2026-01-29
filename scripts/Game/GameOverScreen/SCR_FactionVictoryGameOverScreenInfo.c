[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EGameOverTypes, "m_GameOverScreenId")]
class SCR_FactionVictoryGameOverScreenInfo: SCR_FactionGameOverScreenInfo 
{
	override Color GetVignetteColor(Faction factionPlayer, array<Faction> factionsOther)
	{		
		return factionPlayer.GetFactionColor();
	}
};