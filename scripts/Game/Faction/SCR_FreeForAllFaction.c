//------------------------------------------------------------------------------------------------
class SCR_FreeForAllFaction : SCR_Faction
{
	override bool DoCheckIfFactionFriendly(Faction faction)
	{
		return false;
	}
};
