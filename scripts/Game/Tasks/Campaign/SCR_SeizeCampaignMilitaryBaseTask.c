class SCR_SeizeCampaignMilitaryBaseTaskEntityClass : SCR_CampaignMilitaryBaseTaskEntityClass
{
}

class SCR_SeizeCampaignMilitaryBaseTaskEntity : SCR_CampaignMilitaryBaseTaskEntity
{
	//------------------------------------------------------------------------------------------------
	override void SetMilitaryBaseCallSign(int callSign)
	{
		super.SetMilitaryBaseCallSign(callSign);

		if (!m_MilitaryBase)
			return;

		SCR_GameModeCampaign.GetInstance().GetBaseManager().GetOnSignalChanged().Insert(OnCampaignBaseSignalChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void AwardXP(notnull SCR_XPHandlerComponent handlerXP)
	{
		array<int> players = GetTaskAssigneePlayerIDs();
		foreach (int playerId : players)
		{
			handlerXP.AwardXP(playerId, SCR_EXPRewards.SEIZE_TASK_COMPLETED, 1, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCampaignBaseSignalChanged(SCR_CampaignMilitaryBaseComponent base)
	{
		if (!m_MilitaryBase || base != m_MilitaryBase)
			return;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		bool hasAnyFactionHQSignal = false;
		SCR_CampaignFaction campaignFaction;
		foreach (string factionKey : m_MilitaryBaseTaskData.m_aOwnerFactionKeys)
		{
			if (factionKey.IsEmpty())
				continue;

			campaignFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(factionKey));
			if (!campaignFaction)
				continue;

			if (m_MilitaryBase.IsHQRadioTrafficPossible(campaignFaction))
			{
				hasAnyFactionHQSignal = true;
				break;
			}
		}

		if (!hasAnyFactionHQSignal)
		{
			SetTaskState(SCR_ETaskState.CANCELLED);
			DeleteTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SeizeCampaignMilitaryBaseTaskEntity()
	{
		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		if (!gameMode)
			return;

		SCR_CampaignMilitaryBaseManager baseManager = gameMode.GetBaseManager();
		if (baseManager)
			baseManager.GetOnSignalChanged().Remove(OnCampaignBaseSignalChanged);
	}
}
