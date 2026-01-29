/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class ScriptedWeaponAimModifier: BaseWeaponAimModifier
{
	//------------------------------------------------------------------------------------------------
	event protected void OnInit(IEntity weaponEnt);
	/*!
	Called when owner weapon is enabled via parent owner (taken into hands, manned...)
	\param weaponOwner Entity that activated this weapon
	*/
	event protected void OnActivated(IEntity weaponOwner);
	/*!
	Called when owner weapon is disabled via parent owner (dropped, changed, unmanned ...)
	\param weaponOwner Entity that lost (deactivated) this weapon
	*/
	event protected void OnDeactivated(IEntity weaponOwner);
	/*!
	This method is ticked each frame as long as the aim modifier is active.
	\param owner Owner of this modifier. Not parent manager, but weapon itself.
	\param context Context that defines circumstances for this aim modifier or null if none.
	\param timeSlice Delta time since last update
	\param translation Out translation in meters
	\param rotation Out rotation in degrees in <yaw, pitch, roll> format
	\param turnOffset Out aiming rotation change in degrees <pitch, yaw, unused>
	*/
	event protected void OnCalculate(IEntity owner, WeaponAimModifierContext context, float timeSlice, out vector translation, out vector rotation, out vector turnOffset);
	// called when weapon is fired
	event protected void OnWeaponFired();
}

/*!
\}
*/
