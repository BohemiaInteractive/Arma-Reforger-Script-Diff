/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class ChimeraCharacterClass: GameEntityClass
{
}

class ChimeraCharacter: GameEntity
{
	// Returns HUD display for this character
	proto external BaseInfoDisplay GetInfoDisplay();
	//! Returns ai controller
	proto external AIControlComponent GetAIControlComponent();
	//! Returns component which has animations controlling logic
	proto external CharacterAnimationComponent GetAnimationComponent();
	proto external CharacterCommandHandlerComponent GetCommandHandler();
	//! Returns component which stores information about compartment used by character
	proto external CompartmentAccessComponent GetCompartmentAccessComponent();
	//! Returns component which has character controlling logic
	proto external CharacterControllerComponent GetCharacterController();
	//! Returns component which handles damage
	proto external SCR_DamageManagerComponent GetDamageManager();
	//! Returns component which handles weapons
	proto external BaseWeaponManagerComponent GetWeaponManager();
	//! Returns true if the character is inside a vehicle.
	proto external bool IsInVehicle();
	//! Returns true if the character is in vehicle in ADS
	proto external bool IsInVehicleADS();
	//! Returns the aim position on the character
	proto external vector AimingPosition();
	//! Returns the world position of eyes
	proto external vector EyePosition();
	//! Returns the local position of eyes
	proto external vector EyePositionModel();
	//! Returns character head yaw and pitch in local space (in rads)
	proto external vector GetAimRotationModel();
	//! Returns component which controls head aiming
	proto external AimingComponent GetHeadAimingComponent();
	//! Returns component which controls weapon aiming
	proto external AimingComponent GetWeaponAimingComponent();
	//! Start performing action by the current character
	proto external void DoStartObjectAction(BaseUserAction pAction);
	//! Perform action by current character
	proto external void DoPerformObjectAction(BaseUserAction pAction);
	//! Perform continuous action by current character
	proto external void DoPerformContinuousObjectAction(BaseUserAction pAction, float timeSlice);
	//! Cancel action performed by the current character
	proto external void DoCancelObjectAction(BaseUserAction pAction);
	/*!
	This variant of TraceMove can be used as an optimization if you want to do BaseWorld.TraceMove with a filter that filters out all ChimeraCharacter entities.
	This way, the trace doesn't have to go into script callback to filter out ChimeraCharacter entities.
	This variant doesn't support another filter callback, which would be applied on top of the filtering of ChimeraCharacter entities.
	Traces shape movement along start->end, return 0..1 if trace was sucessfull. Results are filled in param object @see TraceParam
	\param param BaseWorld object. Normally you'd call world.TraceMove(...);
	\param param object used for both input parameters and output parameters @see TraceParam
	\returns	value 0...1, percentage of a path traveled
	*/
	static proto float TraceMoveWithoutCharacters(BaseWorld world, inout TraceParam param);

	// callbacks

	event void OnSpecialContact(IEntity other);
	event void OnSpecialContactsChagned(notnull array<IEntity> contacts);
}

/*!
\}
*/
