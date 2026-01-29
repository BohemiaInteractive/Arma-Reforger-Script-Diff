[BaseContainerProps()]
class SCR_FactionCommanderBaseRequestMenuHandler : SCR_FactionCommanderBaseTaskMenuHandler
{
	[Attribute(desc:"If true, player voted as commander can assign group to request")]
	protected bool m_bCanBeGroupAssignedByCommander;

	protected const string LIMIT_REQUEST_REACHED_TEXT = "#AR-FactionCommander_LimitRequestReached";

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool CanRequestEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		SCR_BaseRequestedTaskEntity task = SCR_BaseRequestedTaskEntity.Cast(hoveredEntity);
		if (task)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanQuickEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanQuickEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		return IsTaskValid(hoveredEntity);
	}

	//------------------------------------------------------------------------------------------------
	bool CanRequestEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		if (GetActiveGroupRequestCount(commanderFaction, playerId) >= SCR_BaseRequestedTaskEntity.REQUEST_MAX_COUNT)
		{
			disabledText = LIMIT_REQUEST_REACHED_TEXT;
			return false;
		}

		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager)
			return false;

		return groupTaskManager.CanCreateNewTaskWithTypename(GetTaskPrefabClassTypename(), commanderFaction);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		if (!super.CanGroupEntryBeShown(commanderFaction, playerId, position, hoveredEntity, group))
			return false;

		// commander can assign group into request if it is allowed by config
		if (m_bCanBeGroupAssignedByCommander && SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		if (!group)
			return false;

		if (GetActiveGroupRequestCount(commanderFaction, playerId) >= SCR_BaseRequestedTaskEntity.REQUEST_MAX_COUNT)
		{
			disabledText = LIMIT_REQUEST_REACHED_TEXT;
			return false;
		}

		SCR_BaseRequestedTaskEntity task = SCR_BaseRequestedTaskEntity.Cast(hoveredEntity);
		if (!task)
		{
			SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
			return groupTaskManager && groupTaskManager.CanCreateNewTaskWithTypename(GetTaskPrefabClassTypename(), commanderFaction);
		}

		SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
		bool canBeAssigned = SCR_TaskSystem.GetInstance().CanTaskBeAssignedTo(task, groupExecutor);
		return canBeAssigned && !task.IsTaskAssignedTo(groupExecutor);
	}

	//------------------------------------------------------------------------------------------------
	//! \return number of active requests of the player group
	protected int GetActiveGroupRequestCount(Faction commanderFaction, int playerId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(playerId);
		if (!playerGroup)
			return false;

		array<SCR_Task> tasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByStateFiltered(
			tasks,
			SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED,
			commanderFaction.GetFactionKey(),
			-1,
			SCR_BaseRequestedTaskEntity,
			true
		);

		int counter;
		SCR_BaseRequestedTaskEntity requestedGroupTask;

		foreach (SCR_Task task : tasks)
		{
			requestedGroupTask = SCR_BaseRequestedTaskEntity.Cast(task);
			if (requestedGroupTask && requestedGroupTask.GetRequesterId() == playerGroup.GetGroupID())
				counter++;
		}

		return counter;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		SCR_GroupsManagerComponent groupComp = SCR_GroupsManagerComponent.GetInstance();
		if (!groupComp)
			return;

		SCR_AIGroup requesterGroup = groupComp.GetPlayerGroup(playerId);
		if (!requesterGroup)
			return;

		if (!IsTaskValid(hoveredEntity))
		{
			factionCommanderPlayerComponent.CreateRequestedTask(GetTaskPrefab(), position, requesterGroup.GetGroupID(), group, playerId, this);
			return;
		}

		if (!SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return;

		SCR_Task task = SCR_Task.Cast(hoveredEntity);
		if (group)
			factionCommanderPlayerComponent.AssignGroupToTask(group.GetGroupID(), task.GetTaskID(), playerId);
	}
}
