[BaseContainerProps()]
class SCR_BuildingTaskCreator : SCR_TaskCreator
{
	[Attribute("10 0 -10", desc:"Task offset")]
	protected vector m_vPositionOffset;

	SCR_CampaignBuildingManagerComponent m_BuildingManager;

	protected static const string CAMPAIGN_BUILDING_TASK_ID = "%1_BuildingCampaignMilitaryBaseTask_%2";

	//------------------------------------------------------------------------------------------------
	protected void OnEntitySpawnedByProvider(int prefabID, SCR_EditableEntityComponent editableEntity, int playerId, SCR_CampaignBuildingProviderComponent provider)
	{
		SCR_CampaignBuildingCompositionComponent buildingCompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!buildingCompositionComponent)
			return;

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!player)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_Faction.GetEntityFaction(player));
		if (!faction)
			return;

		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager || !groupTaskManager.CanCreateNewTaskWithResourceName(m_sTaskPrefab, faction))
			return;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		SCR_CampaignMilitaryBaseComponent base = provider.GetCampaignMilitaryBaseComponent();
		if (!base)
			return;

		string taskID = string.Format(CAMPAIGN_BUILDING_TASK_ID, faction.GetFactionKey(), base.GetCallsign());

		if (SCR_TaskSystem.GetTaskFromTaskID(taskID, false))
			return;

		SCR_BuildingCampaignMilitaryBaseTaskEntity task = SCR_BuildingCampaignMilitaryBaseTaskEntity.Cast(taskSystem.CreateTask(
			m_sTaskPrefab,
			taskID,
			"",
			"",
			base.GetOwner().GetOrigin() + m_vPositionOffset
		));

		if (!task)
		{
			Print("Task was not created", LogLevel.ERROR);
			return;
		}

		task.SetMilitaryBaseCallSign(base.GetCallsign());
		taskSystem.SetTaskOwnership(task, SCR_ETaskOwnership.EXECUTOR);
		taskSystem.SetTaskVisibility(task, SCR_ETaskVisibility.GROUP);
		taskSystem.AddTaskFaction(task, faction.GetFactionKey());

		groupTaskManager.SetGroupTask(task, 0);

		SCR_TaskUIInfo taskUIInfo = task.GetTaskUIInfo();
		if (!taskUIInfo)
			return;

		task.SetTaskName(taskUIInfo.GetName(), {base.GetFormattedBaseNameWithCallsign(faction)});
	}

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_TaskCreatorComponent taskCreatorComponent)
	{
		super.Init(taskCreatorComponent);

		BaseGameMode gameMode = taskCreatorComponent.GetGameMode();
		if (!gameMode)
			return;

		m_BuildingManager = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!m_BuildingManager)
			return;

		m_BuildingManager.GetOnEntitySpawnedByProvider().Insert(OnEntitySpawnedByProvider);
	}

	//------------------------------------------------------------------------------------------------
	override void Deinit()
	{
		if (!m_BuildingManager)
			return;

		m_BuildingManager.GetOnEntitySpawnedByProvider().Remove(OnEntitySpawnedByProvider);
	}
}
