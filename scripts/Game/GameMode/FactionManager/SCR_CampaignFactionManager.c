[BaseContainerProps()]
class SCR_RankIDCampaign : SCR_RankID
{	
	[Attribute("10", UIWidgets.EditBox, "How long does this rank has to wait between requests (sec).", params: "0 inf 1")]
	protected int m_iRequestCD;
	
	[Attribute("30", UIWidgets.EditBox, "Respawn timer when deploying on this unit while it's carrying a radio.", params: "0 inf 1")]
	protected int m_iRadioRespawnCooldown;
	
	[Attribute("300", params: "0 inf 1")]
	protected int m_iFastTravelCooldown;
	
	[Attribute("0", UIWidgets.ComboBox, "ID of this reward.", enums: ParamEnumArray.FromEnum(SCR_ERadioMsg))]
	protected SCR_ERadioMsg m_eRadioMsg;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRankRequestCooldown()
	{
		return m_iRequestCD * 1000;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRankRadioRespawnCooldown()
	{
		return m_iRadioRespawnCooldown;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRankFastTravelCooldown()
	{
		return m_iFastTravelCooldown;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ERadioMsg GetRadioMsg()
	{
		return m_eRadioMsg;
	}
};

class SCR_CampaignFactionManagerClass : SCR_FactionManagerClass
{
}

class SCR_CampaignFactionManager : SCR_FactionManager
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] alliedFaction
	//! \return
	SCR_CampaignFaction GetEnemyFaction(notnull SCR_CampaignFaction alliedFaction)
	{
		array<Faction> factions = {};
		GetFactionsList(factions);
		
		for (int i = factions.Count() - 1; i >= 0; i--)
		{
			SCR_Faction factionCast = SCR_Faction.Cast(factions[i]);
			
			if (factionCast && factionCast.IsPlayable() && factionCast != alliedFaction)
				return SCR_CampaignFaction.Cast(factionCast);
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] factionKey
	//! \return
	SCR_CampaignFaction GetCampaignFactionByKey(string factionKey)
	{
		Faction faction = GetFactionByKey(factionKey);
		if (faction)
			return SCR_CampaignFaction.Cast(faction);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	//! \return
	SCR_CampaignFaction GetCampaignFactionByIndex(int index)
	{
		return SCR_CampaignFaction.Cast(GetFactionByIndex(index));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	int GetRankRequestCooldown(SCR_ECharacterRank rankID)
	{
		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(GetRankByID(rankID));
		
		if (!rank)
			return int.MAX;
		
		return rank.GetRankRequestCooldown();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	int GetRankRadioRespawnCooldown(SCR_ECharacterRank rankID)
	{
		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(GetRankByID(rankID));
		
		if (!rank)
			return int.MAX;
		
		return rank.GetRankRadioRespawnCooldown();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	int GetRankFastTravelCooldown(SCR_ECharacterRank rankID)
	{
		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(GetRankByID(rankID));
		
		if (!rank)
			return int.MAX;

		return rank.GetRankFastTravelCooldown();
	}
}

enum SCR_ECampaignFaction
{
	INDFOR,
	BLUFOR,
	OPFOR
}
