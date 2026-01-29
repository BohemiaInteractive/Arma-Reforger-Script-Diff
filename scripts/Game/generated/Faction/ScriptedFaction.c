/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Faction
\{
*/

class ScriptedFaction: Faction
{
	//! Called when faction is initialized.
	event void Init(IEntity owner);
	/*
	Override this method in inherited class to define faction friendliness.
	Disclaimer: Method can be called very often, especially by AI.
	Queries should be ideally cached and made as fast as possible, so performance is not hindered much.
	*/
	event bool DoCheckIfFactionFriendly(Faction faction);
}

/*!
\}
*/
