/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GameMode
\{
*/

class RespawnSystemComponentClass: GameComponentClass
{
}

//! RespawnSystemComponent should be attached to a gamemode to handle player spawning and respawning.
class RespawnSystemComponent: GameComponent
{
	//! call to create actual entity from given prefab at given position
	//! \param position XYZ position in world space
	//! \param rotation XYZ rotation (pitch, yaw, roll) in world space degrees
	[Obsolete()]
	proto external GenericEntity DoSpawn(string prefab, vector position, vector rotation = "0 0 0");

	// callbacks

	/*!
	Serialize server network data for JIP clients.
	\param w Bit writer instance
	*/
	event protected bool OnRplSave(ScriptBitWriter w) { return true; };
	/*!
	Deserialize data received from server on JIP.
	\param r Bit reader instance
	*/
	event protected bool OnRplLoad(ScriptBitReader r) { return true; };
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	//! Called when some player controller requests respawn
	[Obsolete()]
	event GenericEntity RequestSpawn(int playerId) { return null; };
}

/*!
\}
*/
