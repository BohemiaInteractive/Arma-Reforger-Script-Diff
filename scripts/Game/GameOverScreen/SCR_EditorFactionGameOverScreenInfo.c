[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EGameOverTypes, "m_GameOverScreenId")]
class SCR_EditorFactionGameOverScreenInfo : SCR_FactionGameOverScreenInfo
{		
	[Attribute("", desc: "If m_bGetWinningSubtitleParam is true and multiple factions in array then this text is shown over is subtitle with %1 being player faction.")]
	protected LocalizedString m_sMultiFactionsSubtiteText;
	
	override string GetSubtitle(Faction factionPlayer, array<Faction> factionsOther)
	{
		if (factionsOther.Count() > 1 && m_bGetWinningSubtitleParam)
			return m_sMultiFactionsSubtiteText;
		
		return super.GetSubtitle(factionPlayer, factionsOther);
	}
	
	override string GetSubtitleParam(Faction factionPlayer, array<Faction> factionsOther)
	{
		//Multiple winners but player did not win. So get player faction name instead even if It should get winning faction.
		if (m_bGetWinningFactionImage)
		{
			if (factionsOther.Count() > 1 && !factionsOther.Contains(factionPlayer) && factionPlayer)
			{
				return factionPlayer.GetFactionName();
			}
		}
		
		return super.GetSubtitleParam(factionPlayer, factionsOther);
	}
};