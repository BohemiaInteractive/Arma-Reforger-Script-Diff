[BaseContainerProps()]
class SCR_FactionCommanderEstablishCommandMenuHandler : SCR_FactionCommanderConflictBaseMenuHandler
{
	protected SCR_AIGroup m_Group;
	protected ref SCR_EstablishBaseMapCommandCursor m_MapCommandCursor;

	protected const string ESTABLISH_BASE_LIMIT_REACHED = "#AR-Notification_GROUP_TASK_ESTABLISH_BASE_TOO_MANY_BASES";

	//------------------------------------------------------------------------------------------------
	override bool CanQuickEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanQuickEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		SCR_EstablishBaseTaskEntity task = SCR_EstablishBaseTaskEntity.Cast(hoveredEntity);
		if (!task)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (gameModeCampaign && !gameModeCampaign.GetBaseManager().CanFactionBuildNewBase(commanderFaction))
		{
			disabledText = ESTABLISH_BASE_LIMIT_REACHED;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		SCR_Task task = SCR_EstablishBaseTaskEntity.Cast(hoveredEntity);
		if (!task)
		{
			SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
			if (groupTaskManager && groupTaskManager.CanCreateNewTaskWithTypename(GetTaskPrefabClassTypename(), commanderFaction))
				return true;

			return false;
		}

		SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
		bool canBeAssigned = SCR_TaskSystem.GetInstance().CanTaskBeAssignedTo(task, groupExecutor);
		return canBeAssigned && !task.IsTaskAssignedTo(groupExecutor);
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		SCR_EstablishBaseTaskEntity task = SCR_EstablishBaseTaskEntity.Cast(hoveredEntity);

		if (!task)
		{
			m_MapCommandCursor = new SCR_EstablishBaseMapCommandCursor();
			m_MapCommandCursor.ShowCursor(position);
			m_MapCommandCursor.SetData(commanderFaction, playerId, hoveredEntity);
			m_MapCommandCursor.GetOnCommandExecuted().Insert(OnCommandExecuted);
			m_Group = group;
		}
		else if (group)
			factionCommanderPlayerComponent.AssignGroupToTask(group.GetGroupID(), task.GetTaskID(), playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommandExecuted(vector position)
	{
		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();
		if (!factionCommanderPlayerComponent || !m_MapCommandCursor)
			return;

		factionCommanderPlayerComponent.CreateTask(GetTaskPrefab(), position, m_Group, m_MapCommandCursor.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (gameModeCampaign && !gameModeCampaign.GetEstablishingBasesEnabled())
			return false;

		return super.CanEntryBeShown(commanderFaction, playerId, position, hoveredEntity);
	}
}
