/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup GameMode
\{
*/

class RespawnComponentClass: GameComponentClass
{
}

/*!
Respawn component is a component attached to PlayerController serving as
interface for communication between the authority and remote clients in
respawn system context.
*/
class RespawnComponent: GameComponent
{
	/*!
	Notify the underlying system that this respawn component has spawned/is taking over the provided entity.
	*/
	proto external void NotifySpawn(IEntity spawnedEntity);
	/*!
	Returns the player controller this respawn component is attached to.
	*/
	proto external PlayerController GetPlayerController();

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	/*!
	Called after all components are initialized.
	\param owner Entity this component is attached to.
	*/
	event protected void OnPostInit(IEntity owner);
	/*!
	Called during EOnDiag.
	\param owner Entity this component is attached to.
	\poaram timeSlice Delta time since last update.
	*/
	event protected void OnDiag(IEntity owner, float timeSlice);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnFrame(IEntity owner, float timeSlice);
}

/*!
\}
*/
