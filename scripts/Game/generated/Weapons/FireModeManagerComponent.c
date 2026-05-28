/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapons
\{
*/

class FireModeManagerComponent: GameComponent
{
	proto external IEntity GetOwner();
	proto external TurretControllerComponent GetTurretController();
	proto external void NextWeaponsGroup();
	proto external void NextFireMode();
	proto external void NextRippleCount();
	proto external void RequestSetWeaponsGroup(int weaponsGroupIndex, int rippleModeIndex = 0);

	// callbacks

	event protected void OnWeaponsGroupChanged(int weaponsGroupIndex, int rippleModeIndex);
}

/*!
\}
*/
