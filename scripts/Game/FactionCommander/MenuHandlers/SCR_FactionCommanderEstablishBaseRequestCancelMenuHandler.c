[BaseContainerProps()]
class SCR_FactionCommanderEstablishBaseRequestCancelMenuHandler : SCR_FactionCommanderBaseCancelRequestMenuHandler
{
	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		// for commander is confirm and deny option
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return false;

		return true;
	}
}
