[BaseContainerProps()]
class SCR_FactionCommanderEstablishBaseRequestDenyMenuHandler : SCR_FactionCommanderMainCommandMenuHandler
{
	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		SCR_EstablishBaseRequestedTaskEntity task = SCR_EstablishBaseRequestedTaskEntity.Cast(hoveredEntity);

		return task != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_FactionCommanderPlayerComponent comp = GetLocalCommanderComponent();
		if (!comp)
			return;

		SCR_EstablishBaseRequestedTaskEntity task = SCR_EstablishBaseRequestedTaskEntity.Cast(hoveredEntity);
		if (!task)
			return;

		comp.CancelTask(playerId, task.GetTaskID());
	}
}
