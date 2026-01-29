/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIDangerEventWeaponFire: AIDangerEvent
{
	//! Returns position and direction of the gunshot
	proto external vector GetDirection();
	//! Returns true if suppressor was attached
	proto external bool IsSuppressed();
	//! Returns instigator entity - the entity which pulled the trigger
	proto external IEntity GetInstigatorEntity();
	//! Returns WorldTimestamp when the event was created
	proto external WorldTimestamp GetTimestamp();
	//! Returns initial speed of projectile
	proto external float GetInitialSpeed();
}

/*!
\}
*/
