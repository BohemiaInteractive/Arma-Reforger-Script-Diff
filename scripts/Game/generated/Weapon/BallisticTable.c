/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BallisticTable: ScriptAndConfig
{
	/*!
	Returns aim height and time for next projectile shot with the muzzle at given distance.
	If the distance is further than the max bullet range, it returns negative time and aim height of the projectile max range.
	*/
	static proto float GetAimHeightOfNextProjectile(float distance, out float time, BaseMuzzleComponent muzzleComp, bool bDirectFire = true);
	/*!
	Returns aim height and time for given projectile at given distance. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the distance is further than the max bullet range, it returns negative time and aim height of the projectile max range.
	*/
	static proto float GetHeightFromProjectile(float distance, out float time, IEntity projectile, float initSpeedCoef = 1.0, bool bDirectFire = true);
	/*!
	Returns aim height and time for given projectile source at given distance. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the distance is further than the max bullet range, it returns negative time and aim height of the projectile max range.
	*/
	static proto float GetHeightFromProjectileSource(float distance, out float fTimeResult, IEntitySource projectileSource, float initSpeedCoef = 1.0, bool bDirectFire = true);
	/*
	Retrieves aim hight and time for given projectile at given distance. This method finds the height compensation for the altitude difference between the shooter and the target.
	Returns false, if the projectile with used starting conditions couldn't reach the target distance with the set altitude difference.
	If it gives a non-zero heightResult with returned false, it means that we could reach the altitude but not at that distrance.
	This method currently only works with projectiles with ShellMoveComponent and have undirect fire ballistic table.
	!Warning: This method simulates the projectile, so it takes considerable time to run!
	*/
	static proto bool GetAimHeightOfProjectileAltitudeFromSource(float distance, out float heightResult, out float fTimeResult, IEntitySource projectile, float fAltitudeDifference, float initSpeedCoef = 1.0);
	/*!
	Returns distance and time for given projectile shot at given angle. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the angle is not within the ballistic table range, it returns negative time and distance of the nearest angle projectile .
	*/
	static proto float GetDistanceOfProjectile(float angle, out float time, IEntity projectile, float initSpeedCoef = 1.0, bool bDirectFire = true);
	/*!
	Returns distance and time for given projectile source shot at given angle. This should be used if the projectile is not shot with muzzle. For example for grenades.
	If the angle is not within the ballistic table range, it returns negative time and distance of the nearest angle projectile .
	*/
	static proto float GetDistanceOfProjectileSource(float angle, out float fTimeResult, IEntitySource projectileSource, float initSpeedCoef = 1.0, bool bDirectFire = true);
}

/*!
\}
*/
