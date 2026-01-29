[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EGameOverTypes, "m_GameOverScreenId")]
class SCR_FactionGameOverScreenInfo: SCR_BaseGameOverScreenInfo
{
	[Attribute("1", desc: "If true gets faction flag of factionOther, else gets flag of Player faction. Not if winning faction is true but there are multiple factions then the given Image in optional params is used instead!")]
	protected bool m_bGetWinningFactionImage;
	
	[Attribute("1", desc: "If true gets faction name of factionOther to be subtitle param, else gets name of Player faction")]
	protected bool m_bGetWinningSubtitleParam;
	
	//Get winning faction Image (Or icon if image not found)
	override ResourceName GetImage(Faction factionPlayer, array<Faction> factionsOther)
	{
		SCR_Faction scrFaction;
		
		if (m_bGetWinningFactionImage)
		{
			if (factionsOther.Count() == 1)
			{
				scrFaction = SCR_Faction.Cast(factionsOther[0]);
				if (scrFaction)
					return scrFaction.GetFactionFlag();
				else 			
					return factionsOther[0].GetUIInfo().GetIconPath();
			}
			else if (factionPlayer && factionsOther.Contains(factionPlayer))
			{
				scrFaction = SCR_Faction.Cast(factionPlayer);
				if (scrFaction)
					return scrFaction.GetFactionFlag();
				else 	
					return factionPlayer.GetUIInfo().GetIconPath();
			}
			else if (HasOptionalParams())
			{
				return m_OptionalParams.m_sImageTexture;
			}
		}
		else if (factionPlayer)
		{
			scrFaction = SCR_Faction.Cast(factionPlayer);
			if (scrFaction)
				return scrFaction.GetFactionFlag();
			else 	
				return factionPlayer.GetUIInfo().GetIconPath();
		}
		else if (HasOptionalParams())
		{
			return m_OptionalParams.m_sImageTexture;
		}
			
		return string.Empty;	
	}
	
	//Get winning faction Icon
	override ResourceName GetIcon(Faction factionPlayer, array<Faction> factionsOther)
	{
		SCR_Faction scrFaction;
		
		if (m_bGetWinningFactionImage)
		{
			if (factionsOther.Count() == 1)
			{
				return factionsOther[0].GetUIInfo().GetIconPath();
			}
			else if (factionPlayer && factionsOther.Contains(factionPlayer))
			{
				return factionPlayer.GetUIInfo().GetIconPath();
			}
			else if (HasOptionalParams())
			{
				return m_OptionalParams.m_sIcon;
			}
		}
		else if (factionPlayer)
		{
			return factionPlayer.GetUIInfo().GetIconPath();
		}
		else if (HasOptionalParams())
		{
			return m_OptionalParams.m_sIcon;
		}
			
		return string.Empty;	
	}
	
	
	//Get winning faction name
	override string GetSubtitleParam(Faction factionPlayer, array<Faction> factionsOther)
	{
		if (m_bGetWinningFactionImage)
		{
			if (factionsOther.Count() == 1)
			{
				return factionsOther[0].GetFactionName();
			}
			else if (factionPlayer && factionsOther.Contains(factionPlayer))
			{
				return factionPlayer.GetFactionName();
			}
			else if (HasOptionalParams())
			{
				return m_OptionalParams.m_sSubtitleParam;
			}
		}
		else if (factionPlayer)
		{
			return factionPlayer.GetFactionName();
		}
		else if (HasOptionalParams())
		{
			return m_OptionalParams.m_sSubtitleParam;
		}

		return string.Empty;
	}
};