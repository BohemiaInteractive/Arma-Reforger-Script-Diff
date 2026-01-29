/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class CompartmentAccessComponentClass: GameComponentClass
{
}

class CompartmentAccessComponent: GameComponent
{
	proto external IEntity GetVehicleCompartmentManagerOwner();
	//! Returns the compartent we're in
	proto external BaseCompartmentSlot GetCompartment();
	//! Returns the first free compartment of a given type on \param targetEntity
	proto external BaseCompartmentSlot FindFreeCompartment(IEntity targetEntity, ECompartmentType compartmentType, bool useReserved = true);
	proto external BaseCompartmentSlot FindCompartmentReservedBy(IEntity targetEntity, IEntity reservedBy);
	//! Returns true if we're inside a compartment
	proto external bool IsInCompartment();
	//! Returns true if we're inside a compartment with enabled ADS
	proto external bool IsInCompartmentADS();
	//! Returns true if \param targetEntity is accessible for getting in (e.g. not upside down)
	proto external bool IsTargetVehicleAccessible(IEntity targetEntity);
	//! If force teleport, doorInfoIndex is ignored. closeDoor is still respected.
	proto external bool GetInVehicle(IEntity vehicle, BaseCompartmentSlot compartment, bool forceTeleport, int doorInfoIndex, ECloseDoorAfterActions closeDoor, bool performWhenPaused);
	//! If force teleport, doorInfoIndex is used for the teleport position and closeDoor is ignored.
	proto external bool GetOutVehicle(EGetOutType type, int doorInfoIndex, ECloseDoorAfterActions closeDoor, bool performWhenPaused);
	//! Uses teleport - character is teleported to targetTransform, and door state will not be changed.
	proto external bool GetOutVehicle_NoDoor(vector targetTransform[4], bool sendIntoRagdoll, bool performWhenPaused);
	//! Kick another character out.
	proto external void KickFromVehicle(int doorInfoIndex);
	//! Open door only
	proto external bool OpenDoor(IEntity vehicle, ECharacterDoorAnimType doorAnimType, int doorInfoIndex);
	//! Checks if the door at doorIndex belongs to another vehicle.
	//! \param[in] vehicle Expected owner vehicle
	//! \param[in] doorIndex Valid door index
	proto external bool IsDoorFromAnotherVehicle(IEntity vehicle, int doorIndex);
	//! Checks if the door at doorIndex belongs to another vehicle.
	//! \param[in] vehicle Expected owner vehicle
	//! \param[in] doorIndex Valid door index
	[Obsolete("Use IsDoorFromAnotherVehicle(vehicle, doorIndex).")]
	proto external bool AreDoorsFromDifferentVehicle(IEntity vehicle, int doorIndex);
	//! Close door only
	proto external bool CloseDoor(IEntity vehicle, ECharacterDoorAnimType doorAnimType, int doorInfoIndex);
	//! Interrupt the get in/get out process
	proto external void InterruptVehicleActionQueue(bool interruptCurrentAction, bool forced, bool replicate);
	//! Turn out/in from/to compartment if it has the option for it
	proto external bool TurnOut(bool turnOut);
	//! Returns true if \param targetEntity can be entered at this time
	proto external bool CanGetInVehicle(IEntity targetEntity);
	//! Returns the entity owning the component
	proto external IEntity GetOwner();
	//! Returns true if \param targetEntity can be entered at this time
	proto external bool CanGetInVehicleViaDoor(IEntity targetEntity, BaseCompartmentManagerComponent compartmentManagerCompartment, int doorInfoIndex);
	//! Returns true if \param doorIndex can be closed/opened
	proto external bool CanAccessDoor(IEntity targetEntity, BaseCompartmentManagerComponent compartmentManagerCompartment, int doorInfoIndex);
	//! Returns true while getting in
	proto external bool IsGettingIn();
	//! Returns true while getting out
	proto external bool IsGettingOut();
	//! Returns true if compartment can be gotten out of
	proto external bool CanGetOutVehicle();
	//! Returns true if compartment can be gotten out of via the door with index \param doorIndex
	proto external bool CanGetOutVehicleViaDoor(int doorIndex);
	//! Returns true if character is in the switching animation
	proto external bool IsSwitchingSeatsAnim();
	proto external bool FindSuitableTeleportLocation(vector TR[4]);
	//! Returns true if current compartment can be jumped from
	proto external bool CanJumpOutVehicle();
	proto external bool IsVehicleFlipped(float dotThreshold);
	proto external bool CompareSeatOutVectorAndGravity(float dotThreshold);
	//! Returns the vehicle entity is in (root entity of whole hierarchy)
	static proto IEntity GetVehicleIn(IEntity entity);
	proto external void SetActivePilotCompartment(BaseCompartmentSlot pilotCompartment);

	// callbacks

	event protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move);
	event protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move);
}

/*!
\}
*/
