[BaseContainerProps()]
class SCR_FactionCommanderCancelConflictBaseMenuHandler : SCR_FactionCommanderConflictBaseMenuHandler
{
	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		return GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_CampaignFactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCampaignCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		SCR_Task task = GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
		if (!task)
			return;

		factionCommanderPlayerComponent.CancelTask(playerId, task.GetTaskID());
	}
}
