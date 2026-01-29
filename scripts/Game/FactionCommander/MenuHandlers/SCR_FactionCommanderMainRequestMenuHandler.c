[BaseContainerProps()]
class SCR_FactionCommanderMainRequestMenuHandler : SCR_FactionCommanderBaseMenuHandler
{
	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return true;

		SCR_GroupsManagerComponent comp = SCR_GroupsManagerComponent.GetInstance();
		if (!comp)
			return false;

		SCR_AIGroup group = comp.GetPlayerGroup(playerId);
		if (!group)
			return false;

		return group.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId());
	}
}
