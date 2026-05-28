/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapons
\{
*/

class WeaponsGroup: ScriptAndConfig
{
	void WeaponsGroup();

	proto external int GetAvailableRippleQuantities(notnull out array<int> output);
	proto external int GetNumberOfRippleModes();
	proto external int GetNumberOfWeaponsIndices();
	proto external EWeaponGroupFireMode GetFireMode();
	proto external string GetName();
	proto external int GetWeaponIndices(notnull out array<int> output);
}

/*!
\}
*/
