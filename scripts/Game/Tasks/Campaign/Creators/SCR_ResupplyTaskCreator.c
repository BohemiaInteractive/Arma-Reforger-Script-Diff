[BaseContainerProps()]
class SCR_ResupplyTaskCreator : SCR_TaskCreator
{
	[Attribute("1", desc: "If enabled, resupply tasks are created for ALL playable factions and the Faction Keys list is ignored. If disabled, resupply tasks are created only for factions listed in Faction Keys.")]
	protected bool m_bCreateForAllPlayableFactions;

	[Attribute(desc: "List of faction keys eligible for resupply tasks. Only used if 'CreateForAllPlayableFactions' is disabled.")]
	protected ref array<FactionKey> m_aFactionKeys;

	protected SCR_CampaignMilitaryBaseManager m_MilitaryBaseManager;
	protected ref map<SCR_ResourceConsumer, SCR_CampaignMilitaryBaseComponent> m_mBaseToResourceConsumer = new map<SCR_ResourceConsumer, SCR_CampaignMilitaryBaseComponent>();

	protected static const string CAMPAIGN_RESUPPLY_TASK_ID = "%1_CampaignResupplyTask_%2";

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_TaskCreatorComponent taskCreatorComponent)
	{
		super.Init(taskCreatorComponent);

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		m_MilitaryBaseManager = campaign.GetBaseManager();
		if (!m_MilitaryBaseManager)
			return;

		array<SCR_CampaignMilitaryBaseComponent> bases = {};
		m_MilitaryBaseManager.GetBases(bases);
		foreach (SCR_CampaignMilitaryBaseComponent base : bases)
		{
			SubscribeToBaseEvents(base);
		}

		m_MilitaryBaseManager.GetOnBaseBuilt().Insert(OnBaseBuilt);
		m_MilitaryBaseManager.GetOnSignalChanged().Insert(OnSignalChanged);
		SCR_MilitaryBaseSystem.GetInstance().GetOnBaseFactionChanged().Insert(OnBaseFactionChanged);
		SCR_MilitaryBaseSystem.GetInstance().GetOnBaseUnregistered().Insert(UnsubscribeFromBaseEvents);
	}

	//------------------------------------------------------------------------------------------------
	override void Deinit()
	{
		if (m_MilitaryBaseManager)
		{
			m_MilitaryBaseManager.GetOnBaseBuilt().Remove(OnBaseBuilt);
			m_MilitaryBaseManager.GetOnSignalChanged().Remove(OnSignalChanged);
		}

		foreach (SCR_ResourceConsumer resourceConsumer, SCR_CampaignMilitaryBaseComponent campaignBase : m_mBaseToResourceConsumer)
		{
			UnsubscribeFromBaseEvents(campaignBase);
		}

		SCR_MilitaryBaseSystem militaryBaseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (militaryBaseSystem)
		{
			militaryBaseSystem.GetOnBaseFactionChanged().Remove(OnBaseFactionChanged);
			militaryBaseSystem.GetInstance().GetOnBaseUnregistered().Remove(UnsubscribeFromBaseEvents);
		}

		super.Deinit();
	}

	//------------------------------------------------------------------------------------------------
	protected void SubscribeToBaseEvents(SCR_CampaignMilitaryBaseComponent campaignBase)
	{
		if (!campaignBase)
			return;

		if (campaignBase.GetType() != SCR_ECampaignBaseType.BASE)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(campaignBase.GetFaction());
		if (!faction)
			return;

		if (!campaignBase.IsHQRadioTrafficPossible(faction, SCR_ERadioCoverageStatus.BOTH_WAYS))
			return;

		campaignBase.GetOnSupplyLimitChanged().Insert(OnSuppliesLimitChanged);

		SCR_ResourceConsumer resourceConsumer = campaignBase.GetResourceConsumer();
		if (!resourceConsumer)
			return;

		if (!m_mBaseToResourceConsumer.Contains(resourceConsumer))
			m_mBaseToResourceConsumer.Insert(resourceConsumer, campaignBase);

		resourceConsumer.GetOnResourcesChanged().Insert(OnResourcesChanged);
		resourceConsumer.GetOnMaxResourcesChanged().Insert(OnResourcesChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnsubscribeFromBaseEvents(SCR_MilitaryBaseComponent base)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		if (!campaignBase)
			return;

		if (campaignBase.GetType() != SCR_ECampaignBaseType.BASE)
			return;

		campaignBase.GetOnSupplyLimitChanged().Remove(OnSuppliesLimitChanged);

		SCR_ResourceConsumer resourceConsumer = campaignBase.GetResourceConsumer();
		if (!resourceConsumer)
			return;

		resourceConsumer.GetOnResourcesChanged().Remove(OnResourcesChanged);
		resourceConsumer.GetOnMaxResourcesChanged().Remove(OnResourcesChanged);

		m_mBaseToResourceConsumer.Remove(resourceConsumer);
	}

	//------------------------------------------------------------------------------------------------
	protected void TryCreateResuplyTask(SCR_CampaignMilitaryBaseComponent campaignBase)
	{
		if (!campaignBase || !campaignBase.IsInitialized())
			return;

		if (campaignBase.GetType() != SCR_ECampaignBaseType.BASE)
			return;

		SCR_CampaignFaction faction = campaignBase.GetCampaignFaction();
		if (!faction || !faction.IsPlayable())
			return;

		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager || !groupTaskManager.CanCreateNewTaskWithResourceName(m_sTaskPrefab, faction))
			return;

		if (!m_bCreateForAllPlayableFactions && !m_aFactionKeys.Contains(faction.GetFactionKey()))
			return;

		if (!campaignBase.IsHQRadioTrafficPossible(faction, SCR_ERadioCoverageStatus.RECEIVE))
			return;

		if (!campaignBase.IsResupplyTaskCreationEnabled() || !campaignBase.IsResupplyNeeded())
			return;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		string taskId = string.Format(CAMPAIGN_RESUPPLY_TASK_ID, faction.GetFactionKey(), campaignBase.GetCallsign());
		if (SCR_TaskSystem.GetTaskFromTaskID(taskId, false))
			return;

		vector position = campaignBase.GetOwner().GetOrigin() + {10, 0, 10}; // quick fix - set 10m offset
		SCR_ResupplyCampaignMilitaryBaseTaskEntity newTask = SCR_ResupplyCampaignMilitaryBaseTaskEntity.Cast(taskSystem.CreateTask(
			m_sTaskPrefab, taskId, "", "", position
		));

		if (!newTask)
			return;

		newTask.SetMilitaryBaseCallSign(campaignBase.GetCallsign());
		taskSystem.SetTaskOwnership(newTask, SCR_ETaskOwnership.EXECUTOR);
		taskSystem.SetTaskVisibility(newTask, SCR_ETaskVisibility.GROUP);
		taskSystem.AddTaskFaction(newTask, faction.GetFactionKey());

		SCR_TaskUIInfo taskUIInfo = newTask.GetTaskUIInfo();
		if (!taskUIInfo)
			return;

		newTask.SetTaskName(taskUIInfo.GetName(), { campaignBase.GetFormattedBaseNameWithCallsign(faction) });

		groupTaskManager.SetGroupTask(newTask, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesLimitChanged(SCR_CampaignMilitaryBaseComponent campaignBase, float supplyLimit)
	{
		TryCreateResupplyTaskDelayed(campaignBase);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResourcesChanged(SCR_ResourceInteractor resourceInteractor, float value)
	{
		SCR_ResourceConsumer resourceConsumer = SCR_ResourceConsumer.Cast(resourceInteractor);
		if (!resourceConsumer)
			return;

		SCR_CampaignMilitaryBaseComponent campaignBase = m_mBaseToResourceConsumer.Get(resourceConsumer);
		TryCreateResupplyTaskDelayed(campaignBase);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseBuilt(SCR_CampaignMilitaryBaseComponent campaignBase, Faction faction)
	{
		if (!campaignBase)
			return;

		TryCreateResupplyTaskDelayed(campaignBase);

		SubscribeToBaseEvents(campaignBase);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSignalChanged(SCR_CampaignMilitaryBaseComponent campaignBase)
	{
		if (!campaignBase)
			return;

		TryCreateResupplyTaskDelayed(campaignBase);

		SubscribeToBaseEvents(campaignBase);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseFactionChanged (SCR_MilitaryBaseComponent base, Faction faction)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		if (!campaignBase)
			return;

		TryCreateResupplyTaskDelayed(campaignBase);
	}
	
	protected void TryCreateResupplyTaskDelayed(SCR_CampaignMilitaryBaseComponent campaignBase)
	{
		// I give up - there is so many calllaters in original implementation that I will just use them as well
		GetGame().GetCallqueue().CallLater(TryCreateResuplyTask, SCR_GameModeCampaign.MEDIUM_DELAY, false, campaignBase);
	}
}
