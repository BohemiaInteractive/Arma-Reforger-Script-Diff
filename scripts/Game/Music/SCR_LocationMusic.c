class SCR_LocationMusic : LocationMusic
{
	protected bool m_bPlayerIsInHQ = false;
	
	
	override void Init() 
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		
		if (!baseManager)
			return;
		
		baseManager.GetOnLocalPlayerEnteredBase().Insert(PlayerEnteredHQ);
		baseManager.GetOnLocalPlayerLeftBase().Insert(PlayerLeftHQ);
	}

	override void OnDelete()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		
		if (!baseManager)
			return;
		
		baseManager.GetOnLocalPlayerEnteredBase().Remove(PlayerEnteredHQ);
		baseManager.GetOnLocalPlayerLeftBase().Remove(PlayerLeftHQ);
	}
		
	//~ Player entered their faction HQ
	protected void PlayerEnteredHQ(SCR_CampaignMilitaryBaseComponent base)
	{
		if (m_bPlayerIsInHQ)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
				
		if (!campaign)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_CampaignFaction pFaction = SCR_CampaignFaction.Cast(factionManager.GetLocalPlayerFaction());
		
		if (!pFaction)
			return;
		
		SCR_CampaignMilitaryBaseComponent HQ = pFaction.GetMainBase();
		
		//~ No HQ or the given base is not HQ then ignore
		if (!HQ || base != HQ)
			return;
	
		m_bPlayerIsInHQ = true;
	}
	
	//~ Player left their faction HQ
	protected void PlayerLeftHQ(SCR_CampaignMilitaryBaseComponent base)
	{
		if (!m_bPlayerIsInHQ)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
				
		if (!campaign)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_CampaignFaction pFaction = SCR_CampaignFaction.Cast(factionManager.GetLocalPlayerFaction());
		
		if (!pFaction)
			return;
		
		SCR_CampaignMilitaryBaseComponent HQ = pFaction.GetMainBase();
		
		//~ No HQ or the given base is not HQ then ignore
		if (!HQ || base != HQ)
			return;
		
		m_bPlayerIsInHQ = false;
	}
	
	override bool ShouldPlay()
	{	
		return !m_bPlayerIsInHQ;
	}
}