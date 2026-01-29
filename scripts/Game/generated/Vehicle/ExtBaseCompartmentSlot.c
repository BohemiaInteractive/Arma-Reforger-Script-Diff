/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class ExtBaseCompartmentSlot: ScriptAndConfig
{
	proto external int GetAddUserActions(out notnull array<CompartmentUserAction> outAddUserActions);
	proto external bool IsPiloting();
	proto external bool IsOccupantTurnedOut();
	proto external string GetAutoConnectTurret();
	proto external IEntity GetOwner();
	proto external BaseControllerComponent GetController();
	proto external ECompartmentType GetType();
	proto external BaseCompartmentManagerComponent GetManager();
	proto external int GetAvailableDoorIndices(out notnull array<int> arr);
	proto external bool IsAvailableFromDoor(int doorIndex);
	// returns door index with entry position that is closest to provided point (in world space)
	proto external int PickDoorIndexForPoint(vector point);
	proto external IEntity AttachedOccupant();
	proto external IEntity GetOccupant();
	proto external UIInfo GetUIInfo();
	proto external bool IsReserved();
	proto external bool IsReservedBy(IEntity ent);
	proto external void SetReserved(IEntity ent);
	proto external bool CanTurnOut();
	proto external int GetTurnOutDoorIndex();
	proto external EntitySlotInfo GetPassengerPointInfo();
	proto external bool GetForceFreeLook();
	proto external bool GetMouseSteering();
	// Otherwise they will stay in the compartment and play the unconscious animation.
	proto external bool ShouldEjectUnconsciousDeadCharacters();
	proto external void SetCharacterIsStrappedIn(bool val);
	proto external bool ShouldCharactersFallOutWhenFlipped();
	proto external bool GetCharacterHasCollision();
	proto external void SetCharacterHasCollision(bool val);
	//! Get the object name of the compartment in prefab or unique name if assigned on its properties and bPreferUnique == true.
	proto external string GetCompartmentName(bool bPreferUnique = false);
	//! Returns the manager ID of the compartment. To uniquely identify the compartment, use both GetCompartmentMgrID and GetCompartmentSlotID
	proto external int GetCompartmentMgrID();
	//! Returns the slot ID of the compartment. To uniquely identify the compartment, use both GetCompartmentMgrID and GetCompartmentSlotID
	proto external int GetCompartmentSlotID();
	/*!
	Checks if getting in is impossible for a given entity.
	\warning Covers cases in MP where multiple players could request GetIn.
	The state is reset after getting in is finished (successfully or not).
	\param entity Entity we check the lock against
	\return True if the entity doesn't hold the lock (can't enter)
	*/
	proto external bool IsGetInLockedFor(IEntity entity);
	/*!
	Checks if getting in is locked by anybody.
	\return True if any entity holds the lock (temporarily blocking other from entering)
	*/
	proto external bool IsGetInLocked();
	proto external CompartmentUserAction GetGetInAction();
	proto external CompartmentUserAction GetGetOutAction();
	proto external CompartmentUserAction GetJumpOutAction();
	proto external CompartmentUserAction GetSwitchSeatAction();
	proto external vector GetPosition();
	proto external PointInfo GetOverridenEntryPointInfo();
	proto external PointInfo GetOverridenExitPointInfo();
	proto external bool ShouldSkipCharacterDoorAnimationsFromSeat();
	proto external string GetCompartmentUniqueName();
	proto external void AttachTurret(TurretControllerComponent turretController, ETurretContextID contextIDs);
	proto external void DetachTurret();
	proto external TurretControllerComponent GetAttachedTurret();

	// callbacks

	// Called on DiagDebugDrawPosition
	event protected void DebugDrawPosition();
}

/*!
\}
*/
