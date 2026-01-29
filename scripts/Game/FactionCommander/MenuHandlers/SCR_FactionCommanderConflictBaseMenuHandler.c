[BaseContainerProps()]
class SCR_FactionCommanderConflictBaseMenuHandler : SCR_FactionCommanderBaseTaskMenuHandler
{
	[Attribute("1", desc:"If checked, player commander can create objective")]
	protected bool m_bIsObjectiveCreationEnabled;

	protected const string BASE_NOT_SELECTED_TEXT = "#AR-FactionCommander_NoBaseSelected";

	//------------------------------------------------------------------------------------------------
	bool CanCreateObjectiveEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return m_bIsObjectiveCreationEnabled;
	}

	//------------------------------------------------------------------------------------------------
	bool CanCreateObjectiveEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager)
			return false;

		return groupTaskManager.CanCreateNewTaskWithTypename(GetTaskPrefabClassTypename(), commanderFaction);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		if (!GetHoveredBase())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CampaignFactionCommanderPlayerComponent GetLocalCampaignCommanderComponent()
	{
		return SCR_CampaignFactionCommanderPlayerComponent.Cast(GetLocalCommanderComponent());
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CampaignMilitaryBaseComponent GetHoveredBase()
	{
		SCR_CampaignFactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCampaignCommanderComponent();

		if (!factionCommanderPlayerComponent)
			return null;

		return factionCommanderPlayerComponent.GetHoveredBase();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Task GetTaskOnBase(SCR_CampaignMilitaryBaseComponent targetBase, Faction targetFaction, typename taskClass)
	{
		if (!targetBase)
			return null;

		array<SCR_Task> tasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByState(tasks, SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED, targetFaction.GetFactionKey());
		SCR_CampaignMilitaryBaseTaskData data;
		SCR_CampaignMilitaryBaseComponent base;

		foreach (SCR_Task task : tasks)
		{
			SCR_CampaignMilitaryBaseTaskEntity campaignMilitaryBaseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(task);
			if (!campaignMilitaryBaseTask)
				continue;

			data = SCR_CampaignMilitaryBaseTaskData.Cast(task.GetTaskData());
			if (!data)
				continue;

			if (data.m_aOwnerFactionKeys.IsEmpty())
				continue;

			base = campaignMilitaryBaseTask.GetMilitaryBase();
			if (!base)
				continue;

			if (task.Type() == taskClass && base == targetBase && data.m_aOwnerFactionKeys.Contains(targetFaction.GetFactionKey()))
				return task;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Task GetTask(IEntity hoveredEntity, notnull Faction faction, typename taskClass)
	{
		SCR_Task task = SCR_Task.Cast(hoveredEntity);
		if (!task || task && task.Type() != taskClass)
			task = GetTaskOnHoveredBase(faction, taskClass);

		return task;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Task GetTaskOnHoveredBase(notnull Faction faction, typename taskClass)
	{
		return GetTaskOnBase(GetHoveredBase(), faction, taskClass);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanCampaignGroupEntryBeSelected(notnull Faction faction, IEntity hoveredEntity, notnull SCR_AIGroup group, typename taskClass)
	{
		SCR_Task task = GetTask(hoveredEntity, faction, taskClass);
		if (!task)
		{
			SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
			if (groupTaskManager && groupTaskManager.CanCreateNewTaskWithTypename(taskClass, faction))
				return true;

			return false;
		}

		SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
		bool canBeAssigned = SCR_TaskSystem.GetInstance().CanTaskBeAssignedTo(task, groupExecutor);
		return canBeAssigned && !task.IsTaskAssignedTo(groupExecutor);
	}
}
