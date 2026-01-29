/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile
\{
*/

class ProjectileMoveComponent: BaseProjectileComponent
{
	proto external void EnableSimulation(IEntity owner);
	proto external void Launch(vector direction, vector parentVelocity, float initSpeedCoef, IEntity projectileEntity, IEntity gunner, IEntity parentEntity, IEntity lockedTarget, IEntity weaponComponent);
	/*!
	* Predicts the projectile simulation result (there's no interaction with the world)
	*
	*/
	proto external vector GetProjectileSimulationResult(vector initPosition, float initSpeed, float initElevationAngle, float initAzimuth = 0, vector windSpeed = Vector(0,0,0), float targetHeight = 0, float maxSimulationTime = 0);
	proto external vector GetVelocity();
	/*!
	* Queries the rewind duration (equal to the RTT of the remote shooter's connection at the time of the shot) from the projectile for lag compensation.
	* \warning This has to be performed on authority, and before a shell is launched or on non-network shots, the return value will be zero.
	* \return The current rewind duration in seconds.
	*/
	proto external float GetRewindDuration();
	/*!
	* Sets the rewind duration (equal to the RTT of the remote shooter's connection) of the projectile for lag compensation.
	* \param duration The new rewind duration in milliseconds.
	* \warning This has to be performed on authority, and must be done after the shell is launched.
	*/
	proto external void SetRewindDuration(float duration);
	/*!
	* Gets the bullet speed coef of this bullet. Init speed gets created with muzzle bullet speed coef and this bullet speed coef. (Used for i.e. mortar charge rings)
	* \return Bullet speed coef.
	* \warning Value does not return total speed coef but just the value set on the bullet. For total: multiply bullet speed coef from muzzle with this value.
	*/
	proto external float GetBulletSpeedCoef();
	/*!
	* Sets the bullet speed coef of this bullet. Init speed gets created with muzzle bullet speed coef and this bullet speed coef. (Used for i.e. mortar charge rings)
	* \param coef New bullet speed coef
	* \warning Value does not set total speed coef but just the value set on the bullet. For total: multiply bullet speed coef from muzzle with this value.
	*/
	proto external void SetBulletCoef(float coef);
}

/*!
\}
*/
