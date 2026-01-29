/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseWeaponManagerComponentClass: GameComponentClass
{
}

class BaseWeaponManagerComponent: GameComponent
{
	ref ScriptInvoker<BaseWeaponComponent> m_OnWeaponChangeCompleteInvoker = new ScriptInvoker<BaseWeaponComponent>();
	ref ScriptInvoker<BaseWeaponComponent> m_OnWeaponChangeStartedInvoker = new ScriptInvoker<BaseWeaponComponent>();

	proto external IEntity GetOwner();
	//! performs throwing
	proto external void Throw(vector vDirection, float fSpeedScale);
	proto external int GetDefaultWeaponIndex();
	//! Returns currently selected weapon or null if none. (This can either be a weapon component directly or a slot component.)
	proto external BaseWeaponComponent GetCurrent();
	proto external BaseWeaponComponent GetCurrentWeapon();
	proto external BaseWeaponComponent GetCurrentGrenade();
	proto external WeaponSlotComponent GetCurrentSlot();
	proto external GrenadeSlotComponent GetCurrentGrenadeSlot();
	proto external SightsComponent GetCurrentSights();
	proto external int GetCurrentSightsIndex();
	/*!
	Select the weapon passed by the parameter. Returns true if the weapon has been selected.
	*IMPORTANT* This is not synchronized in multiplayer, it will only replicate during streams-in correctly. You will need to implement your RPC to send this command to every client.
	*/
	proto external bool SelectWeapon(BaseWeaponComponent weapon);
	//! Returns the old weapon entity of the specified weapon slot
	proto external IEntity SetSlotWeapon(WeaponSlotComponent pSlot, IEntity pWeaponEntity);
	//! Returns false if there's no weapon that this manager can select other than the one that's already selected.
	proto external bool HasAnotherSelectableWeapon();
	//! weapons visibility
	proto external void SetVisibleAllWeapons(bool state);
	proto external void SetVisibleCurrentWeapon(bool state);
	/*!
	Returns the list of slots.
	\param outSlots The array where all slots will be inserted.
	\return The number of slots in the list
	*/
	proto external int GetWeaponsSlots(out notnull array<WeaponSlotComponent> outSlots);
	/*!
	Returns the list of weapons. These can be either slots or weapons.
	\param outWeapons The array with all weapons managed by this manager.
	\return The number of weapons in the list
	*/
	proto external int GetWeapons(out notnull array<BaseWeaponComponent> outWeapons);
	/*!
	Returns the list of weapons.
	\param outWeapons The array where all weapons will be inserted.
	\return The number of weapons in the list
	*/
	proto external int GetWeaponsList(out notnull array<IEntity> outWeapons);
	//! Returns true if current weapon is valid and outputs current weapon's muzzle transformation to the given matrix.
	proto external bool GetCurrentMuzzleTransform(vector outMatrix[4]);
	//! Returns true if current weapon is valid and has some sights data, outputs sights transformation and fov
	proto bool GetCurrentSightsTransform(out vector outWorldMatrix[4], out vector outLocalMatrix[4], out float fov);
	//! Returns true if current weapon is valid and has some sights data, outputs local sights transformation with adjusted roll angle and fov
	proto bool GetCurrentSightsCameraTransform(out vector outLocalMatrix[4], out float fov);

	// callbacks

	event protected void OnWeaponChangeComplete(BaseWeaponComponent newWeaponSlot) { m_OnWeaponChangeCompleteInvoker.Invoke(newWeaponSlot); };
	event protected void OnWeaponChangeStarted(BaseWeaponComponent newWeaponSlot) { m_OnWeaponChangeStartedInvoker.Invoke(newWeaponSlot); };
}

/*!
\}
*/
