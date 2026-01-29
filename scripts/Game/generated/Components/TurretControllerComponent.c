/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class TurretControllerComponent: CompartmentControllerComponent
{
	proto external BaseCompartmentSlot GetCompartmentSlot();
	//! Returns true if we can only aim in ADS.
	proto external bool GetCanAimOnlyInADS();
	//! Returns true if the free look is enabled.
	proto external bool IsFreeLookEnabled();
	//! Returns true if the weapon is in ADS.
	proto external bool IsWeaponADS();
	//! Request weapon ADS state
	proto external void SetWeaponADS(bool val);
	//! Returns weapon ADS input state
	proto external bool GetWeaponADSInput();
	//! Request weapon ADS input state
	proto external void SetWeaponADSInput(bool val);
	//! Enable or disable weapon ADS
	proto external void SetWeaponADSEnabled(bool val);
	proto external bool GetWeaponADSEnabled();
	proto external ETurretReloadState GetReloadingState();
	//! Returns the time to ADS in seconds.
	proto external float GetADSTime();
	proto external BaseSightsComponent GetCurrentSights();
	proto external bool GetCurrentSightsADS();
	/*!
	Set if we are currently in ADS.
	\param on True if we are ADS otherwise false.
	*/
	proto external void SetCurrentSightsADS(bool on);
	//! Returns true if current turret is valid and has some sights data, outputs world sights transformation and FOV
	proto bool GetCurrentSightsCameraTransform(out vector outWorldMatrix[4], out float fov);
	//! Returns true if current turret is valid and has some sights data, outputs local sights transformation and FOV
	proto bool GetCurrentSightsCameraLocalTransform(out vector outLocalMatrix[4], out float fov);
	proto external bool AssembleTurret();
	proto external bool DisassembleTurret();
	//! Remove the given weapon index, and put it into pReceiver's inventory. If pReceiver is null, drop it to the ground. pUser can be nullptr if called from server
	proto external bool RemoveWeapon(IEntity user, int index, IEntity receiver);
	//! Add the weapon to the turret on the given slot index. pUser can be nullptr if called from server
	proto external bool AddWeapon(IEntity user, int index, IEntity weapon);
	proto external TurretComponent GetTurretComponent();
	proto external BaseWeaponManagerComponent GetWeaponManager();
	proto external InventoryStorageManagerComponent GetInventoryManager();
	/*!
	Select the new weapon specified in parameter.
	\param user The user will select the weapon.
	\param newWeapon The new weapon that will be selected.
	\return Returns true if the selection has happened.
	*/
	proto external bool SelectWeapon(IEntity user, BaseWeaponComponent newWeapon);
	/*!
	Select a group of weapons for firing
	\param weapons An array of weapon slots to use (corresponding to the indices defined in WeaponSlotComponents)
	\param mode One of EWeaponGroupFireMode, WGFM_SALVO, WGFM_RIPPLE or WGFM_SEQUENTIAL
	\param rippleQuantity (optional) How many weapons to fire in ripple mode (default 1)
	\param timeBetween (optional) time between ripples (in ms, default 100) or sequential firing (if WGFM_SEQUENTIAL)
	Note that calling SelectWeapon or passing in an empty array will return to normal firing of single weapons
	*/
	proto external void SetWeaponGroup(array<int> weapons, EWeaponGroupFireMode mode, int rippleQuantity = 1, float timeBetween = 100.0);
	// Returns the reload duration in seconds.
	proto external float GetReloadDuration();
	// Returns the current reloading time, it goes from reload duration to 0.
	proto external float GetReloadTime();
	// Instantly reloads weapon with magazine
	proto external bool DoReloadWeaponWith(IEntity ammunitionEntity);
	//! Set if turret is firing or not
	proto external void SetFireWeaponWanted(bool val);
	/*!
	Set aiming angles in radians.
	*IMPORTANT* Only works locally. Used mainly for cinematic
	*/
	proto external void SetAimingAngles(float yaw, float pitch);
	proto external bool GetIsOverridden();
	//! Returns the context indices that are used when this turret is connected to another compartment slot.
	proto external ETurretContextID GetContextIDs();
	proto external string GetUniqueName();
	proto external bool GetUseVehicleCamera();
	/*!
	Used to remotely trigger weapon actions on the turret. Note that the
	turret must be overridden for this to have any effect
	*/
	proto external void SetWeaponInputState(ETurretWeaponInputState state);

	// callbacks

	event protected void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player);
}

/*!
\}
*/
