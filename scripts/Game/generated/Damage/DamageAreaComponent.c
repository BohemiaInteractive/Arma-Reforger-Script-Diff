/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Damage
\{
*/

class DamageAreaComponentClass: ScriptComponentClass
{
}

class DamageAreaComponent: ScriptComponent
{
	proto external void RegisterDamageArea(notnull DamageArea area);
	proto external void UnregisterDamageArea(notnull DamageArea area);
	/*!
	Clears passed array (damageAreas) and fills it with all damageAreas owned by this component. Regardless if they have been registered or not
	\param array<DamageArea> damageAreas: An array that will be filled
	*/
	proto external void GetDamageAreas(out notnull array<ref DamageArea> damageAreas);
}

/*!
\}
*/
