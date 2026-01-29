[BaseContainerProps()]
class SCR_FactionCommanderCancelMenuHandler : SCR_FactionCommanderBaseTaskMenuHandler
{
	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		if (!IsTaskValid(hoveredEntity))
			return false;

		// commander can confirm all tasks if it is allowed by config
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_Task task = SCR_Task.Cast(hoveredEntity);
		if (!task)
			return;

		if (!IsTaskValid(hoveredEntity))
			return;

		SCR_FactionCommanderPlayerComponent comp = GetLocalCommanderComponent();
		if (!comp)
			return;

		comp.CancelTask(playerId, task.GetTaskID());
	}
}
