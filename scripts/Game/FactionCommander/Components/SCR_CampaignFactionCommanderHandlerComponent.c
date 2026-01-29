//! Handles voting and AI decisions for the faction Commander role in Conflict. Should be attached to gamemode entity.
[ComponentEditorProps(category: "GameScripted/Commander", description: "Handles voting and AI decisions for the faction Commander role. Should be attached to gamemode entity.")]
class SCR_CampaignFactionCommanderHandlerComponentClass : SCR_FactionCommanderHandlerComponentClass
{
}

class SCR_CampaignFactionCommanderHandlerComponent : SCR_FactionCommanderHandlerComponent
{
	[Attribute("100", desc: "How far from the Establish task will players still be able to build a new base.", params: "0 inf 1")]
	protected int m_iBaseEstablishingRadius;

	[Attribute()]
	protected ref SCR_MinBaseDistanceConfig m_MinBaseDistanceConfig;

	static const string SEIZE_TASK_RELAY_NAME = "#AR-CampaignTasks_TitleHack-UC";
	static const string SEIZE_TASK_RELAY_DESC = "#AR-CampaignTasks_DescriptionReconfigure";
	static const string SEIZE_TASK_NAME = "#AR-CampaignTasks_TitleSeize-UC";
	static const string SEIZE_TASK_DESC = "#AR-CampaignTasks_DescriptionSeize";
	static const string SEIZE_TASK_ID = "%1_SeizeCampaignMilitaryBase_%2";
	static const ResourceName SEIZE_TASK_PREFAB = "{1CE0D63C852E747B}Prefabs/Tasks/SeizeCampaignMilitaryBaseTaskEntity.et";

	//------------------------------------------------------------------------------------------------
	int GetBaseEstablishingRadius()
	{
		return m_iBaseEstablishingRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	//! \return min base distance
	int GetBaseMinDistance(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		return GetBaseMinDistance(base.GetType());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseType
	//! \return min base distance
	int GetBaseMinDistance(SCR_ECampaignBaseType baseType)
	{
		if (!m_MinBaseDistanceConfig)
			return 0;

		return m_MinBaseDistanceConfig.GetMinDistanceFromBase(baseType);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseFactionChanged(SCR_MilitaryBaseComponent base, Faction faction)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

		if (!campaignBase)
			return;

		SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(faction);

		if (!campaignFaction || !campaignFaction.IsPlayable())
			return;

		GetGame().GetCallqueue().CallLater(CheckSeizeTasks, SCR_GameModeCampaign.DEFAULT_DELAY, false, campaignFaction);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRadioCoverageChanged(notnull SCR_CoverageRadioComponent radio)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(radio.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));

		// We care only about bases
		if (!base)
			return;

		SCR_CampaignFaction faction = base.GetCampaignFaction();

		if (!faction)
		{
			GetGame().GetCallqueue().CallLater(OnRadioCoverageChanged, SCR_GameModeCampaign.MEDIUM_DELAY, false, radio);
			return;
		}

		// used call later, because base callsings are not set yet
		GetGame().GetCallqueue().CallLater(CheckSeizeTasks, SCR_GameModeCampaign.DEFAULT_DELAY, false, faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommanderChanged(SCR_Faction faction, int commanderPlayerId)
	{
		SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(faction);
		if (!campaignFaction)
			return;

		CheckSeizeTasks(campaignFaction);

		if (!campaignFaction.IsAICommander())
			campaignFaction.SendHQMessage(SCR_ERadioMsg.COMMANDER_ARRIVAL);
		else
			campaignFaction.SendHQMessage(SCR_ERadioMsg.COMMANDER_RESIGNATION);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckSeizeTasks(notnull SCR_CampaignFaction faction)
	{
		SCR_MilitaryBaseSystem militaryBaseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!militaryBaseSystem)
			return;

		array<SCR_MilitaryBaseComponent> bases = {};
		militaryBaseSystem.GetBases(bases);

		array<Faction> factions = {};
		GetGame().GetFactionManager().GetFactionsList(factions);

		SCR_CampaignFaction campaignFaction;
		SCR_CampaignMilitaryBaseComponent campaignbase;
		SCR_CampaignFaction baseFaction;
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignbase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (!campaignbase || !campaignbase.IsValidForSeizeTask())
				continue;

			baseFaction = campaignbase.GetCampaignFaction();
			if (!baseFaction)
				continue;

			foreach (Faction f : factions)
			{
				campaignFaction = SCR_CampaignFaction.Cast(f);
				if (!campaignFaction || !campaignFaction.IsPlayable() || !campaignFaction.IsAICommander())
					continue;

				if (baseFaction == campaignFaction)
					continue;

				if (!campaignbase.IsHQRadioTrafficPossible(campaignFaction))
					continue;

				CreateSeizeTask(campaignFaction, campaignbase);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create seize task
	//! \param[in] faction
	//! \param[in] targetBase
	void CreateSeizeTask(notnull SCR_CampaignFaction faction, SCR_CampaignMilitaryBaseComponent targetBase)
	{
		if (GetTaskOnBase(targetBase, faction, SCR_SeizeCampaignMilitaryBaseTaskEntity))
			return;

		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager || !groupTaskManager.CanCreateNewTaskWithResourceName(SEIZE_TASK_PREFAB, faction))
			return;

		string taskId = string.Format(SEIZE_TASK_ID, faction.GetFactionKey(), targetBase.GetCallsign());
		string title;
		string description;

		if (targetBase.GetType() == SCR_ECampaignBaseType.RELAY)
		{
			title = SEIZE_TASK_RELAY_NAME;
			description = SEIZE_TASK_RELAY_DESC;
		}
		else
		{
			title = SEIZE_TASK_NAME;
			description = SEIZE_TASK_DESC;
		}

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		SCR_CampaignMilitaryBaseTaskEntity newTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(taskSystem.CreateTask(
			SEIZE_TASK_PREFAB,
			taskId,
			"",
			description,
			targetBase.GetOwner().GetOrigin()
		));

		if (!newTask)
			return;

		newTask.SetTaskName(title, {targetBase.GetFormattedBaseNameWithCallsign(faction)});
		newTask.SetMilitaryBaseCallSign(targetBase.GetCallsign());
		taskSystem.SetTaskOwnership(newTask, SCR_ETaskOwnership.EXECUTOR);
		taskSystem.SetTaskVisibility(newTask, SCR_ETaskVisibility.GROUP);
		taskSystem.AddTaskFaction(newTask, faction.GetFactionKey());

		groupTaskManager.SetGroupTask(newTask, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Task GetTaskOnBase(notnull SCR_CampaignMilitaryBaseComponent campaignMilitaryBase, notnull Faction faction, typename taskClass)
	{
		array<SCR_Task> tasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByStateFiltered(
			tasks,
			SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED,
			faction.GetFactionKey(),
			-1,
			taskClass
		);

		SCR_CampaignMilitaryBaseTaskData data;
		SCR_CampaignMilitaryBaseComponent base;
		SCR_CampaignMilitaryBaseTaskEntity campaignMilitaryBaseTask;

		foreach (SCR_Task task : tasks)
		{
			campaignMilitaryBaseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(task);
			if (!campaignMilitaryBaseTask)
				continue;

			data = SCR_CampaignMilitaryBaseTaskData.Cast(task.GetTaskData());
			if (!data)
				continue;

			base = campaignMilitaryBaseTask.GetMilitaryBase();
			if (!base)
				continue;

			if (base == campaignMilitaryBase)
				return task;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Player XP changed, award commander a fraction of gained XP if eligible for the reward
	//! \param[in] playerId
	//! \param[in] currentXP
	//! \param[in] XPToAdd
	//! \param[in] rewardId
	protected void OnPlayerXPChanged(int playerId, int currentXP, int XPToAdd, SCR_EXPRewards rewardId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(playerId);
		if (!playerGroup)
			return;

		// To prevent duplicate addition of reward XP, we only reward Commander XP for Squad Leader gained XP
		if (playerGroup.GetLeaderID() != playerId)
			return;

		SCR_Faction playerFaction = SCR_Faction.Cast(playerGroup.GetFaction());
		if (!playerFaction)
			return;

		// AI Commander is not awarded any XP
		if (playerFaction.IsAICommander())
			return;

		int commanderId = playerFaction.GetCommanderId();
		AwardCommanderTaskCompletionXP(commanderId, rewardId, XPToAdd);
	}

	//------------------------------------------------------------------------------------------------
	//! Awards a fraction of gained player XP to Commander
	//! The amount of XP added is determined by the original amount of XP multiplied by Commander Reward XP Amount Fraction
	//! \param[in] commander player Id
	//! \param[in] rewardId
	//! \param[in] XPToAdd - the original amount of XP gained
	protected void AwardCommanderTaskCompletionXP(int commanderId, SCR_EXPRewards rewardId, int XPToAdd)
	{
		SCR_XPHandlerComponent xpHandler = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (!xpHandler)
			return;

		SCR_CommanderAdditionalXPRewardInfo rewardInfo = SCR_CommanderAdditionalXPRewardInfo.Cast(xpHandler.GetXpRewardInfo(SCR_EXPRewards.COMMANDER_TASK_COMPLETED));
		if (!rewardInfo)
			return;

		// Only a subset of XP awards should award Commander XP as well
		if (!rewardInfo.IsXPRewardListed(rewardId))
			return;

		xpHandler.AwardXP(commanderId, SCR_EXPRewards.COMMANDER_TASK_COMPLETED, 1.0, false, rewardInfo.GetRewardXPAmountFraction() * XPToAdd); 
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		RegisterPlayerXPInvokers(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		RegisterPlayerXPInvokers(playerId, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnServerCommanderChanged(int newCommanderId, int oldCommanderId)
	{
		super.OnServerCommanderChanged(newCommanderId, oldCommanderId);

		RegisterPlayerXPInvokers(newCommanderId, true);
		RegisterPlayerXPInvokers(oldCommanderId, false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilled(instigatorContextData);

		int playerID = instigatorContextData.GetVictimPlayerID();

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (!faction)
			return;

		if (faction.IsPlayerCommander(playerID))
			faction.SendHQMessage(SCR_ERadioMsg.COMMANDER_DISRUPTION);
	}

	//------------------------------------------------------------------------------------------------
	// Registers or unregisters invoker on player xp change
	// \param[in] playerId
	// \param[in] unregister - if true, invokers are unregistered, if false, invokers are registered
	protected void RegisterPlayerXPInvokers(int playerId, bool unregister = false)
	{
		if (playerId <= 0)
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;

		SCR_PlayerXPHandlerComponent xpHandler = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!xpHandler)
			return;

		if (unregister)
			xpHandler.GetOnPlayerXPChanged().Remove(OnPlayerXPChanged);
		else
			xpHandler.GetOnPlayerXPChanged().Insert(OnPlayerXPChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		SCR_RadioCoverageSystem radioSystem = SCR_RadioCoverageSystem.GetInstance();
		if (!radioSystem)
			return;

		radioSystem.GetOnCoverageChanged().Insert(OnRadioCoverageChanged);

		SCR_MilitaryBaseSystem baseSystem = SCR_MilitaryBaseSystem.GetInstance();

		if (!baseSystem)
			return;

		baseSystem.GetOnBaseFactionChanged().Insert(OnBaseFactionChanged);

		GetOnFactionCommanderChanged().Insert(OnCommanderChanged);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignFactionCommanderHandlerComponent()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_RadioCoverageSystem radioSystem = SCR_RadioCoverageSystem.GetInstance();
		if (!radioSystem)
			return;

		radioSystem.GetOnCoverageChanged().Remove(OnRadioCoverageChanged);

		SCR_MilitaryBaseSystem baseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseSystem)
			return;

		baseSystem.GetOnBaseFactionChanged().Remove(OnBaseFactionChanged);

		GetOnFactionCommanderChanged().Remove(OnCommanderChanged);
	}
}
