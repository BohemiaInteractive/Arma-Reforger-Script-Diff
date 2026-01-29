[BaseContainerProps()]
class SCR_FactionCommanderMainCommandMenuHandler : SCR_FactionCommanderBaseMenuHandler
{
	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander();
	}
}
