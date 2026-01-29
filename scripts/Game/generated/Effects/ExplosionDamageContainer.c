/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Effects
\{
*/

class ExplosionDamageContainer: BaseProjectileEffect
{
	//Entities inside of this list will not block explosions nor take damage from them. Set the ignore list before the projectile trigger is activated
	proto external void SetIgnoreList(array<IEntity> ignoreList);
	proto external void GetIgnoreList(out array<IEntity> outIgnoreList);
}

/*!
\}
*/
