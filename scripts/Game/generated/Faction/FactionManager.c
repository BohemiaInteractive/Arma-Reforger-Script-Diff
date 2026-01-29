/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Faction
\{
*/

class FactionManagerClass: GenericEntityClass
{
}

class FactionManager: GenericEntity
{
	/*
	\brief Tries to find a faction stored in the manager by name.
	\return Returns a faction or null if faction does not exist.
	*/
	proto external Faction GetFactionByKey(FactionKey factionKey);
	/*
	\brief Tries to find a faction stored in the manager by index.
	\return Returns a faction or null if faction with provided index does not exist.

	Note: Keep in mind that due to internal storage of factions the index might not
	correspond to the originally defined order! Use GetFactionByKey instead, if neccessary.
	*/
	proto external Faction GetFactionByIndex(int index);
	/*
	\brief Finds provided faction and returns its index.
	\return Returns faction index or -1 if action does not exist or is null.

	Note: Keep in mind that due to internal storage of factions the index might not
	correspond to the originally defined order! Use GetFactionByKey instead, if neccessary.
	*/
	proto external int GetFactionIndex(Faction faction);
	/*!
	Returns the number of available factions stored in this manager or 0 if none.
	\return Returns the number of factions
	*/
	proto external int GetFactionsCount();
	/*
	\brief Fills an array with factions stored in the FactionManager
	\param outFactions Output array that will be cleared and then filled with factions
	\return Returns the number of factions that were added to the provided list
	*/
	proto external int GetFactionsList(out notnull array<Faction> outFactions);
}

/*!
\}
*/
