class SCR_CampaignSourceBaseComponentClass : SCR_CampaignMilitaryBaseComponentClass
{
}

class SCR_CampaignSourceBaseComponent : SCR_CampaignMilitaryBaseComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void HandleSpawnPointFaction()
	{
		if (!m_SpawnPoint)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;
		
		super.HandleSpawnPointFaction();
		SCR_CampaignFaction spawnFaction = SCR_CampaignFaction.Cast(m_FactionComponent.GetAffiliatedFaction());
		if (spawnFaction && spawnFaction.CanSpawnOnSourceBases() && spawnFaction.IsPlayable())
		{
			m_SpawnPoint.SetSpawnPointEnabled_S(true);
		}
		else
		{
			m_SpawnPoint.SetSpawnPointEnabled_S(false);
			m_SpawnPoint.SetFactionKey(FactionKey.Empty);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected int GetSuppliesIncomeAmount()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return 0;

		if (m_iRegularSuppliesIncomeBase > -1)
			return m_iRegularSuppliesIncomeBase;

		return campaign.GetRegularSuppliesIncomeSource();
	}

	//------------------------------------------------------------------------------------------------
	override protected int GetSuppliesArrivalTimer()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return int.MAX;

		if (m_iSuppliesArrivalInterval > -1)
			return m_iSuppliesArrivalInterval;

		return campaign.GetSuppliesArrivalIntervalSource();
	}
}
