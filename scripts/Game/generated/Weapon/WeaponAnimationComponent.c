/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class WeaponAnimationComponent: BaseItemAnimationComponent
{
	proto external bool GetBipod();
	proto external bool HasBipod();
	proto external void SetBipod(bool open, bool fastForwardAnimations = false);
	proto external void FoldWeapon(bool fastForwardAnimations = false);
	proto external void UnfoldWeapon(bool fastForwardAnimations = false);
	proto external bool IsFolded();
}

/*!
\}
*/
