[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sFactionKey", "%1 EstablishBaseAIFactionCommander")]
class SCR_EstablishBaseAIFactionCommander : SCR_BaseAIFactionCommander
{
	[Attribute("8", desc:"AI Response time [s]", params: "0 inf")]
	protected float m_fResponseTime;

	[Attribute("", UIWidgets.ResourceNamePicker, "Establish base task", "et")]
	protected ResourceName m_sEstablishBaseTaskPrefab;

	protected SCR_GroupTaskManagerComponent m_GroupTaskManager;
	protected SCR_TaskSystem m_TaskSystem;

	//------------------------------------------------------------------------------------------------
	override protected void OnAICommanderActivated()
	{
		super.OnAICommanderActivated();

		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!gameModeCampaign || !gameModeCampaign.GetEstablishingBasesEnabled())
			return;

		if (!m_GroupTaskManager || !m_TaskSystem)
			return;

		m_GroupTaskManager.GetOnRequestedGroupTaskSet().Insert(OnRequestedGroupTaskSet);

		array<SCR_Task> tasks = {};
		m_TaskSystem.GetTasksByStateFiltered(tasks, SCR_ETaskState.CREATED, m_sFactionKey, -1, SCR_EstablishBaseRequestedTaskEntity, false);

		SCR_EstablishBaseRequestedTaskEntity requestTask;
		int requestedTaskCount;

		// check all establish base requests and create objective for them
		foreach (SCR_Task task : tasks)
		{
			requestTask = SCR_EstablishBaseRequestedTaskEntity.Cast(task);
			if (!requestTask)
				continue;

			requestedTaskCount++;

			// call later is used to delay the AI response
			GetGame().GetCallqueue().CallLater(CreateTask, m_fResponseTime * 1000 * requestedTaskCount, false, requestTask);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnAICommanderDeactivated()
	{
		super.OnAICommanderDeactivated();

		if (!m_GroupTaskManager)
			return;

		m_GroupTaskManager.GetOnRequestedGroupTaskSet().Remove(OnRequestedGroupTaskSet);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRequestedGroupTaskSet(notnull SCR_Task task, int byPlayerID, int requesterGroupId)
	{
		if (!m_bIsAICommanderActivated)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(byPlayerID));
		if (!faction || faction != m_Faction)
			return;

		SCR_EstablishBaseRequestedTaskEntity requestTask = SCR_EstablishBaseRequestedTaskEntity.Cast(task);
		if (requestTask)
			GetGame().GetCallqueue().CallLater(CreateTask, m_fResponseTime * 1000, false, requestTask);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateTask(SCR_EstablishBaseRequestedTaskEntity requestTask)
	{
		// check if AI is still commander after delay
		if (!m_bIsAICommanderActivated)
			return;

		if (!requestTask || !m_TaskSystem)
			return;

		if (!m_GroupTaskManager.CanCreateNewTaskWithResourceName(m_sEstablishBaseTaskPrefab, m_Faction))
			return;

		vector taskPosition = requestTask.GetTaskPosition();
		string taskID = string.Format(SCR_FactionCommanderPlayerComponent.TASK_ID, m_sFactionKey, SCR_FactionCommanderPlayerComponent.GenerateTaskID());

		SCR_Task task = m_TaskSystem.CreateTask(m_sEstablishBaseTaskPrefab, taskID, "", "", taskPosition);
		if (!task)
		{
			Print("Task was not created", LogLevel.ERROR);
			return;
		}

		m_TaskSystem.SetTaskOwnership(task, SCR_ETaskOwnership.EXECUTOR);
		m_TaskSystem.SetTaskVisibility(task, SCR_ETaskVisibility.GROUP);
		m_TaskSystem.AddTaskFaction(task, m_sFactionKey);

		SCR_TaskUIInfo taskUIInfo = task.GetTaskUIInfo();
		if (!taskUIInfo)
			return;

		task.SetTaskName(taskUIInfo.GetName(), {SCR_MapEntity.GetGridLabel(taskPosition)});

		if (m_GroupTaskManager)
			m_GroupTaskManager.SetGroupTask(task, SCR_Faction.AI_COMMANDER_ID);
	}

	//------------------------------------------------------------------------------------------------
	override void Init(notnull SCR_FactionCommanderHandlerComponent factionCommanderHandlerComponent)
	{
		m_TaskSystem = SCR_TaskSystem.GetInstance();
		m_GroupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();

		super.Init(factionCommanderHandlerComponent);
	}

	//------------------------------------------------------------------------------------------------
	override void Deinit()
	{
		super.Deinit();

		if (!m_GroupTaskManager)
			return;

		m_GroupTaskManager.GetOnRequestedGroupTaskSet().Remove(OnRequestedGroupTaskSet);
	}
}
