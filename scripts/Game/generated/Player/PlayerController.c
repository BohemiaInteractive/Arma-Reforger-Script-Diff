/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Player
\{
*/

class PlayerControllerClass: GenericControllerClass
{
}

class PlayerController: GenericController
{
	proto external IEntity GetControlledEntity();
	/*!
	Set the new controlled entity for the player.
	Ignored if called from clients in MP.
	*/
	proto external bool SetControlledEntity(IEntity entity);
	proto external PlayerCamera GetPlayerCamera();
	/*!
	When a PlayerController.RequestRespawn() is issued, the controller engages an internal lock
	that waits for response from the authority or timeout and drops any additional requests.

	This method returns whether we can currently issue a request respawn or not.
	\return True in case that we have no internal lock engaged and we can request, false otherwise.
	*/
	[Obsolete("Use SCR_RespawnComponent.CanSpawn instead!")]
	proto external bool CanRequestRespawn();
	//! Send request to the server to spawn a playable controller for us.
	[Obsolete("Use SCR_RespawnComponent.RequestSpawn instead!")]
	proto external void RequestRespawn();
	//! Returns the Action Manager associated to this player controller
	proto external ActionManager GetActionManager();
	/*!
	Returns the RespawnComponent attached to this PlayerController or null if none.
	\return RespawnComponent instance or null if none.
	*/
	proto external RespawnComponent GetRespawnComponent();
	proto external HUDManagerComponent GetHUDManagerComponent();
	//! Returns text communication privilege
	[Obsolete("Use SocialComponent.IsPrivilegedTo().")]
	proto external bool IsChatAllowed();
	//! Returns voice communication privilege
	[Obsolete("Use SocialComponent.IsPrivilegedTo().")]
	proto external bool IsVonAllowed();
	//! Returns True if the user has given role assigned
	proto external bool HasRole(EPlayerRole role);
	proto external bool SetCharacterCameraRenderActive(bool active);
	proto external int GetPlayerId();
	proto external int GetRplIdentity();
	proto external GamepadIOHandlerComponent GetGamepadIOHandlerComponent();
	proto external void ResetGamepadIOHandler();
	proto external void ApplyGamepadEffectImmediate(GamepadEffect effect);

	// callbacks

	event protected void OnInit(IEntity owner);
	/*
	Event raised during ownership changes.

	The process is as following:
		1. Notify the owner that ownership is about to change.
			Event is raised as changing=true and becameOwner=!IsOwner
			This way we can prepare for the ownership change, e.g. if there are any queued inputs we can process them first with all IsOwner checks as they are.
		2. The owner sends and acknowledgment to the server
		3. The server performs the same thing as owner in (1).
		4. The server passes the ownership.
			Event is raised as changing=false, at this point becameOwner always reflects real state.
	*/
	event protected void OnOwnershipChanged(bool changing, bool becameOwner);
	//! Runs every time the controlled entity has been changed.
	event protected void OnControlledEntityChanged(IEntity from, IEntity to);
	//! Runs every time the controlled entity die.
	event protected void OnDestroyed(notnull Instigator killer);
	event void OnUpdate(float timeSlice);
}

/*!
\}
*/
