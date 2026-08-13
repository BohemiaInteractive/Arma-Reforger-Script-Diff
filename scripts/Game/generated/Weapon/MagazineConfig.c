/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class MagazineConfig: ScriptAndConfig
{
	//! Clears, then fills an array with the ammo ResourceNames
	//! Returns the number of entries in the array
	proto external int GetAmmoResourceArray(out notnull array<ResourceName> ammoResourceNames);
	//! Reads magazine config object from given file
	static proto ref MagazineConfig ReadMagazineConfig(ResourceName fileName);
}

/*!
\}
*/
