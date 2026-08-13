/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_System
\{
*/

//! Request to spawn entities or scripted states from the data known to persistence system.
class PersistenceSpawnRequest: Managed
{
	PersistenceCollection Collection;
	ref array<UUID> Include;
	ref array<UUID> Exclude;
	int Limit;
	int Offset;

}

/*!
\}
*/
