[BaseContainerProps()]
class SCR_FactionCommanderCommandConflictMenuHandler : SCR_FactionCommanderConflictBaseMenuHandler
{
	//------------------------------------------------------------------------------------------------
	override bool CanQuickEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanQuickEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		return IsTaskValid(hoveredEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		SCR_Task task = GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
		if (!m_bIsObjectiveCreationEnabled && !task)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		if (GetHoveredBase())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		if (!super.CanGroupEntryBeSelected(commanderFaction, playerId, position, hoveredEntity, group))
			return false;

		return CanCampaignGroupEntryBeSelected(commanderFaction, hoveredEntity, group, GetTaskPrefabClassTypename());
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_FactionCommanderPlayerComponent comp = GetLocalCommanderComponent();
		if (!comp)
			return;

		if (!IsTaskValid(hoveredEntity))
		{
			comp.CreateTask(GetTaskPrefab(), position, group, playerId, this);
			return;
		}

		SCR_Task task = SCR_Task.Cast(hoveredEntity);
		if (group)
			comp.AssignGroupToTask(group.GetGroupID(), task.GetTaskID(), playerId);
	}
}
