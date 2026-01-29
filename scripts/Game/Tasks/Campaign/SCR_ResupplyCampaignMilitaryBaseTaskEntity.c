class SCR_ResupplyCampaignMilitaryBaseTaskEntityClass : SCR_CampaignMilitaryBaseTaskEntityClass
{
}

class SCR_ResupplyCampaignMilitaryBaseTaskEntity : SCR_CampaignMilitaryBaseTaskEntity
{
	protected SCR_ResourceComponent m_BaseResourceComponent;
	protected SCR_ResourceGenerator m_BaseResourceGenerator;

	//------------------------------------------------------------------------------------------------
	override protected void OnTargetBaseCaptured(SCR_MilitaryBaseComponent base, Faction faction)
	{
		if (!m_MilitaryBase || base != m_MilitaryBase)
			return;

		SetTaskState(SCR_ETaskState.CANCELLED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	override void SetMilitaryBaseCallSign(int callSign)
	{
		super.SetMilitaryBaseCallSign(callSign);

		if (!m_MilitaryBase)
			return;

		SCR_ResourceConsumer consumerComponent = m_MilitaryBase.GetResourceConsumer();
		if (!consumerComponent)
			return;

		consumerComponent.GetOnResourcesChanged().Insert(OnResourcesChanged);
		m_MilitaryBase.GetOnSupplyLimitChanged().Insert(OnSupplyLimitChanged);

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign || !campaign.IsMaster())
			return;

		campaign.GetOnGameEnd().Insert(OnGameEnd);

		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return;

		baseManager.GetOnSignalChanged().Insert(OnSignalChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSignalChanged(SCR_CampaignMilitaryBaseComponent campaignBase)
	{
		if (!m_MilitaryBase || campaignBase != m_MilitaryBase)
			return;

		SCR_CampaignFaction faction = campaignBase.GetCampaignFaction();
		if (m_MilitaryBase.IsHQRadioTrafficPossible(faction, SCR_ERadioCoverageStatus.BOTH_WAYS))
			return;

		SetTaskState(SCR_ETaskState.CANCELLED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGameEnd()
	{
		UnregisterEvents();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResourcesChanged(SCR_ResourceInteractor interactor, float previousSupplies)
	{
		TryFinishTaskDelayed();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSupplyLimitChanged(SCR_CampaignMilitaryBaseComponent campaignBase, float supplyLimit)
	{
		TryFinishTaskDelayed();
	}

	//------------------------------------------------------------------------------------------------
	protected void TryFinishTask()
	{
		if (!m_MilitaryBase)
			return;

		if (m_MilitaryBase.IsResupplyNeeded())
			return;

		SetTaskState(SCR_ETaskState.COMPLETED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void TryFinishTaskDelayed()
	{
		// Delay to avoid Recursive call of Invoke from ResourceConsumer
		GetGame().GetCallqueue().Call(TryFinishTask);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterEvents()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign && campaign.IsMaster())
		{
			campaign.GetOnGameEnd().Remove(OnGameEnd);

			SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
			if (!baseManager)
			{
				baseManager.GetOnSignalChanged().Remove(OnSignalChanged);
			}
		}

		if (!m_MilitaryBase)
			return;

		SCR_ResourceConsumer consumerComponent = m_MilitaryBase.GetResourceConsumer();
		if (!consumerComponent)
			return;

		consumerComponent.GetOnResourcesChanged().Remove(OnResourcesChanged);
		m_MilitaryBase.GetOnSupplyLimitChanged().Remove(OnSupplyLimitChanged);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ResupplyCampaignMilitaryBaseTaskEntity()
	{
		UnregisterEvents();
	}
}
