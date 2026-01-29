/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Damage
\{
*/

class BaseDamageContext: Managed
{
	// damage type
	EDamageType damageType;
	// amount of damage
	float damageValue;
	// damaged entity
	IEntity hitEntity = null;
	// collider ID - if exists
	int colliderID;
	// hitzone to damage
	HitZone struckHitZone;
	//projectile
	IEntity damageSource;
	//instigator
	ref Instigator instigator;
	// hit surface physics material
	GameMaterial material;
	// hit position
	vector hitPosition;
	// hit direction
	vector hitDirection;
	// hit surface normal
	vector hitNormal;
	// projectile velocity in time of impact
	vector impactVelocity;
	// bone index in mesh obj
	int boneIndex;
	// DamageEffect
	ref BaseDamageEffect damageEffect;

}

/*!
\}
*/
