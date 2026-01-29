/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseMagazineComponent: GameComponent
{
	proto external IEntity GetOwner();
	proto external bool IsUsed();
	/*!
	*IMPORTANT* Could only be called on the master.
	Set the remaining ammo.
	*/
	proto external void SetAmmoCount(int ammoCount);
	//! Returns the remaining ammo
	proto external int GetAmmoCount();
	//! Returns the maximum amount of ammo in this magazine
	proto external int GetMaxAmmoCount();
	proto external BaseMagazineWell GetMagazineWell();
	proto external UIInfo GetUIInfo();
}

/*!
\}
*/
