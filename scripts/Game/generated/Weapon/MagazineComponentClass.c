/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class MagazineComponentClass: BaseMagazineComponentClass
{
	//! Returns the magazine config
	proto external MagazineConfig GetMagazineConfig();
	//! Clears, then fills an array with the ammo mappings
	//! Returns amount of entries in the array
	proto external int GetAmmoMapping(out notnull array<int> ammoMappings);
}

/*!
\}
*/
