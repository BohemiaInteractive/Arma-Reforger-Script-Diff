/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class RocketEjectorMuzzleComponent: BaseMuzzleComponent
{
	//! Returns true if no projectile is loaded in the barrel. False otherwise.
	proto external bool CanReloadBarrel(int iBarrelIndex);
	//! Reloads the barrel if possible.
	//! Server-side only. Ignored on clients.
	proto external void ReloadBarrel(int iBarrelIndex, IEntity reloadEntity);
	//! Returns true if any unloaded barrel is found. False otherwise.
	proto external bool CanReloadNextBarrel();
	//! Reloads the first non-empty barrel if possible.
	//! Server-side only. Ignored on clients.
	proto external void ReloadNextBarrel(IEntity reloadEntity);
	//! Returns true if the provided entity is compatible as ammo.
	proto external bool IsAmmoCompatible(IEntity reloadEntity);
	proto external BaseEjectorInterface GetEjectorInterface();
	//!	Returns the projectile entity loaded in barrel iBarrelIndex
	//!	WARNING: Do not modify or delete the projectile entity.
	//!          It is meant to be read-only for users.
	proto external IEntity GetBarrelProjectile(int iBarrelIndex);
	//! Unloads the barrel with index iBarrelIndex.
	//! Server-side only. Ignored on clients.
	proto external void UnloadBarrel(int iBarrelIndex);
	//! Get an array of all loaded entities. The output array will have one entry for each barrel
	//! \param entities A non-null array to hold the result. Will be overwritten
	//! \return Number of Barrels
	proto external int GetLoadedEntities(out notnull array<IEntity> entities);
	//! Set the next barrel index to be fired. Can set the next barrel index to an empty barrel
	//! \param The index of the barrel
	//! \return true if the operation was successful, false if the barrel index is out of range
	proto external bool SetBarrelIndex(int barrelIndex);
}

/*!
\}
*/
