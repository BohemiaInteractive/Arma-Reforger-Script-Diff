/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseMuzzleComponent: GameComponent
{
	// Returns true if muzzle is suppressed
	proto external bool IsMuzzleSuppressed();
	proto external EMuzzleType GetMuzzleType();
	proto external int GetNextFireModeIndex();
	proto external int GetFireModesCount();
	proto external int GetFireModeIndex();
	proto external int GetFireModesList(out notnull array<BaseFireMode> outFireModes);
	proto external BaseFireMode GetCurrentFireMode();
	proto external BaseFireMode GetFireMode(int fireModeIndex);
	proto external void SetTriggerEffect(int fireModeIndex, bool bEnable, WeaponGamepadEffectsManagerComponent pEffectsManager, bool isCharged = true);
	proto external void SetFiringTriggerEffectEnabled(int fireModeIndex, bool bEnable, WeaponGamepadEffectsManagerComponent pEffectsManager);
	proto external float GetBulletInitSpeedCoef();
	// Barrels
	proto external int GetBarrelsCount();
	proto external int GetCurrentBarrelIndex();
	proto external int GetLastBarrelFired();
	proto external bool IsBarrelChambered(int barrelIndex);
	proto external bool IsCurrentBarrelChambered();
	proto external bool IsChamberingPossible();
	//Deletes the bullet from selected chamber as well as Invoking OnAmmoCountChanged
	proto external bool ClearChamber(int barrelIndex);
	// Disposable
	proto external bool IsDisposable();
	// Magazines
	proto external int GetAmmoCount();
	proto external int GetMaxAmmoCount();
	proto external BaseMagazineComponent GetMagazine();
	proto external BaseMagazineWell GetMagazineWell();
	// Returns ResourceName of default magazine if the muzzle takes magazines or ResourceName of default projectile if muzzle takes projectiles.
	proto external ResourceName GetDefaultMagazineOrProjectileName();
	// UI Info
	proto external UIInfo GetUIInfo();
	// Open Bolt Info
	proto external bool GetOpenBoltState();
	// Get gamepad effect context name
	proto external owned string GetGamepadEffectContext();
	proto external void SetGamepadEffectsEnabled(bool val);
	proto external WeaponGamepadEffectsManagerComponent GetGamepadEffectsManager();
	proto external IEntity GetOwner();
}

/*!
\}
*/
