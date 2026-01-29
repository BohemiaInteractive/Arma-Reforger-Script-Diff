/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterInputContext
{
	// MOVEMENT
	proto void GetMovement(out float pSpeed, out vector pLocalDirection);
	proto external void SetMovement(float speed, vector pLocalDirection);
	//! GetStanceChange - returns the currently happening stance change.
	proto external ECharacterStanceChange GetStanceChange();
	//! returns 0 (no roll), (1 - left), (2 - right)
	proto external int GetRollCommand();
	proto external void SetRoll(int rollCommand);
	//! 0 - no roll, 1 - right, -1 - left
	proto external float GetLean();
	proto external void SetLean(float lean);
	proto external bool IsMoving();
	proto external bool IsSprinting();
	proto external bool IsAdjustedStanceEnabled();
	proto external void ToggleAdjustedStance(bool enabled);
	//! Set the aim change in degree
	proto external void SetAimChange(vector aimChange);
	//! Get the aim change in degree
	proto external vector GetAimChange();
	//! returns 0..2pi angle of heading (rotation around world Y axis)
	proto external float GetHeadingAngle();
	proto external void SetHeadingAngle(float newHeadingAngle);
	//! Set the aiming angle in radians
	proto external void SetAimingAngles(vector aimingAngles);
	//! Get the aiming angle in radians
	proto external vector GetAimingAngles();
	//! Set the aiming angle in radians
	proto external void SetLookAtAngles(vector lookAtAngles);
	//! Get the aiming angle in radians
	proto external vector GetLookAtAngles();
	// set weapon translation
	proto external void SetWeaponIKTranslation(vector pWeaponTrans);
	proto external vector GetWeaponIKTranslation();
	//! is there a free look ?
	proto external bool CameraIsFreeLook();
	proto external void SetFreelook(bool pFreelook);
	proto external void SetTransferFreeaimAfterFreelookRequest(bool val);
	proto external bool ShouldTransferFreeaimAfterFreelook();
	proto external void AccumulateFreelookAdjustment(vector change);
	//! is weapon raised
	proto external bool WeaponIsRaisedOrADS();
	proto external bool WeaponIsRaised();
	proto external void SetRaiseWeapon(bool bRaise);
	proto external void SetWeaponPullTrigger(bool bPull);
	proto external bool WeaponIsPullingTrigger();
	proto external bool WeaponIsStartReloading();
	proto external int GetWeaponReloadType();
	proto external void SetReloadWeapon(int ReloadType);
	proto external void SetWeaponADS(bool ads);
	proto external bool IsWeaponADS();
	proto external bool IsWeaponObstructionEnabled();
	proto external void ToggleWeaponObstruction(bool enabled);
	// Actions
	proto external void SetJump(float val);
	proto external float GetJump();
	proto external void SetThrow(bool val);
	proto external bool GetThrow();
	proto external void CancelThrow();
	proto external bool IsThrowCanceled();
	proto external void SetDie(bool val, float direction);
	proto external float GetDie();
	proto external void SetMeleeAttack(bool val);
	proto external bool GetMeleeAttack();
	proto external void SetHit(EHitReactionType hitRecationType, float direction);
	proto external EHitReactionType GetHitReaction();
	proto external float GetHitDirection();
	proto external void SetVehicleDoorToggle(bool toggle);
	proto external bool GetVehicleDoorToggle();
	proto external void SetVehicleTurnOutToggle(bool toggle);
	proto external bool GetVehicleTurnOutToggle();
	proto external bool GetVehicleTurnOut();
	proto external void SetVehicleTurnOut(bool val);
	proto external bool GetVehicleTurnIn();
	proto external void SetVehicleTurnIn(bool val);
	proto external void SetVehicleDoorInfoIndex(int val);
	proto external int GetVehicleDoorInfoIndex();
	proto external void SetVehicleTeleportTarget(vector target[]);
	proto external vector[] GetVehicleTeleportTarget();
	proto external void SetVehicleSteering(float val);
	proto external float GetVehicleSteering();
	proto external void SetVehicleThrottle(float val);
	proto external float GetVehicleThrottle();
	proto external void SetVehicleClutch(float val);
	proto external float GetVehicleClutch();
	proto external void SetVehicleBrake(float val);
	proto external float GetVehicleBrake();
	proto external void SetVehicleCollective(float val);
	proto external float GetVehicleCollective();
	proto external void SetVehicleCyclicForward(float val);
	proto external float GetVehicleCyclicForward();
	proto external void SetVehicleCyclicAside(float val);
	proto external float GetVehicleCyclicAside();
	proto external void SetVehicleAntiTorque(float val);
	proto external float GetVehicleAntiTorque();
	proto external int SetVehicleGearSwitch();
	proto external void GetVehicleGearSwitch(int val);
	proto external void SetVehicleEngineStartStop(int val);
	proto external int GetVehicleEngineStartStop();
	proto external void SetVehicleLights(int val);
	proto external int GetVehicleLights();
	proto external void SetVehicleHorn(int val);
	proto external int GetVehicleHorn();
	proto external void SetVehicleHandBrake(EHandBrakeType val);
	proto external EHandBrakeType GetVehicleHandBrake();
	// Gestures/Animations
	proto external bool IsPlayingGesture();
	proto external void SetPlayingGesture(bool val);
	proto external int GetGestureID();
	proto external void SetGestureID(int gestureID);
	proto external bool IsUnconscious();
	// Returns the entity only if the gadget is in the hand - not when it is hidden by another action.
	proto external IEntity GetLeftHandGadgetEntity();
	// Returns the entity both when the gadget is in hand and when hidden by another action.
	proto external IEntity GetWantedLefHandGadgetEntity();
	proto external void SetCancelItemAction(bool value);
	// Returns depth of the water body in which character is standing, up to 2m (computed from a raycast going directly down from the character).
	proto external float GetWaterBodyApproxDepth();
	// Returns how deep the character is submerged in water (how deep the entity point is). Is influenced by character animations.
	proto external float GetWaterCharacterSubmersion();
	// Returns how deep the character's nose is submerged in water. Is influenced by character animations.
	proto external float GetWaterNoseSubmersion();
	// Returns the elevation of the water surface in world space.
	proto external float GetWaterBodySurfaceY();
}

/*!
\}
*/
