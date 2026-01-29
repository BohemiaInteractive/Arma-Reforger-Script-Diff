/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class MortarMuzzleComponent: MuzzleInMagComponent
{
	proto external void LoadMortar(IEntity shell);

	// callbacks

	event void OnPostInit(IEntity owner);
	event void EOnInit(IEntity owner);
}

/*!
\}
*/
