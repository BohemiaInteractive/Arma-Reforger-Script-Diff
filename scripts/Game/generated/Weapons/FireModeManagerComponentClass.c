/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapons
\{
*/

class FireModeManagerComponentClass: GameComponentClass
{
	proto external int GetNumberOfWeaponsGroups();
	proto external bool GetWeaponsGroup(int weaponGroupIndex, notnull out WeaponsGroup grp);
	proto external int GetNumberOfWeaponsIndices(int weaponGroupIndex);
	proto external int GetNumberOfRippleCounts(int weaponGroupIndex);
	proto external EWeaponGroupFireMode GetFireMode(int weaponGroupIndex);
	proto external string GetWeaponsGroupName(int weaponGroupIndex);
	proto external int GetWeaponIndices(int weaponGroupIndex, notnull out array<int> output);
	proto external int GetAvailableRippleQuantities(int weaponGroupIndex, notnull out array<int> output);
	proto external int GetAvailableFireModes(notnull out array<EWeaponGroupFireMode> output);
}

/*!
\}
*/
