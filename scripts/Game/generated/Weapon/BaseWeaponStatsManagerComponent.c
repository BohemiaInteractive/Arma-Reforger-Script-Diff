/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseWeaponStatsManagerComponentClass: GameComponentClass
{
}

class BaseWeaponStatsManagerComponent: GameComponent
{
	proto external IEntity GetOwner();
	proto external int GetNumberOfMuzzles();
	//! Muzzle Velocity Coefficient
	proto external bool SetMuzzleVelocityCoefficient(IEntity attach, int iMuzzleIndex, float muzzleCoeff);
	proto external bool ClearMuzzleVelocityCoefficient(IEntity attach, int iMuzzleIndex);
	proto bool GetMuzzleVelocityCoefficient(int iMuzzleIndex, out float coeff);
	//! Shot Sound
	proto external bool SetShotSoundOverride(IEntity attach, int iMuzzleIndex, bool bOverride);
	proto external bool ClearShotSoundOverride(IEntity attach, int iMuzzleIndex);
	proto bool GetShotSound(int iMuzzleIndex, out bool bOveride);
	//! Muzzle Effects
	proto external bool SetMuzzleEffectOverride(IEntity attach, int iMuzzleIndex, bool bOverrde);
	proto external bool AddMuzzleEffectOverride(IEntity attach, int iMuzzleIndex, MuzzleEffectComponent muzzleEffect);
	proto external bool ClearMuzzleEffectOverride(IEntity attach, int iMuzzleIndex);
	proto bool GetMuzzleEffectOverride(int iMuzzleIndex, out bool bOverride);
	//! Dispersion Diameter
	proto external bool SetMuzzleDispersionFactor(IEntity attach, int iMuzzleIndex, float fFactor);
	proto external bool ClearMuzzleDispersionFactor(IEntity attach, int iMuzzleIndex);
	proto bool GetMuzzleDispersionFactor(int iMuzzleIndex, out float fFactor);
	//! Weapon extra obstruction length
	proto external bool SetExtraObstructionLength(IEntity attach, float extraLength);
	proto external bool ClearExtraObstructionLength(IEntity attach);
	proto bool GetExtraObstructionLength(out float extraLength);
	//! Is Bayonet. Mostly used for stabby-stabby anim
	proto external bool SetIsBayonet(IEntity attach, bool bIsBayonet);
	proto external bool ClearIsBayonet(IEntity attach);
	proto bool GetIsBayonet(out bool bIsBayonet);
	//! Melee Damage Factor
	proto external bool SetMeleeDamageFactor(IEntity attach, float fDamageFactor);
	proto external bool ClearMeleeDamageFactor(IEntity attach);
	proto bool GetMeleeDamageFactor(out float fDamageFactor);
	//! Melee Range Factor
	proto external bool SetMeleeRangeFactor(IEntity attach, float fRangeFactor);
	proto external bool ClearMeleeRangeFactor(IEntity attach);
	proto bool GetMeleeRangeFactor(out float fRangeFactor);
	//! Melee Accuracy Factor
	proto external bool SetMeleeAccuracyFactor(IEntity attach, float fAccuracyFactor);
	proto external bool ClearMeleeAccuracyFactor(IEntity attach);
	proto bool GetMeleeAccuracyFactor(out float fAccuracyFactor);
	//! Recoil Linear Override Factors
	proto external bool SetRecoilLinearFactors(IEntity attach, int iMuzzleIndex, vector vLinearFactors);
	proto external bool ClearRecoilLinearFactors(IEntity attach, int iMuzzleIndex);
	proto bool GetRecoilLinearFactors(int iMuzzleIndex, out vector vLinearFactors);
	//! Recoil Angular Override Factors
	proto external bool SetRecoilAngularFactors(IEntity attach, int iMuzzleIndex, vector vAngularFactors);
	proto external bool ClearRecoilAngularFactors(IEntity attach, int iMuzzleIndex);
	proto bool GetRecoilAngularFactors(int iMuzzleIndex, out vector vAngularFactors);
	//! Recoil Turn Override Factors
	proto external bool SetRecoilTurnFactors(IEntity attach, int iMuzzleIndex, vector vTurnFactors);
	proto external bool ClearRecoilTurnFactors(IEntity attach, int iMuzzleIndex);
	proto bool GetRecoilTurnFactors(int iMuzzleIndex, out vector vTurnFactors);

	// callbacks

	/*!
	* Called when entity is being attached, after it was attached.
	* iMuzzleIndex specifies the index of the muzzle it was attached
	* to, or -1 if the entity was not attached to a muzzle
	*/
	event void OnWeaponAttachedAttachment(IEntity entity, int iMuzzleIndex);
	/*!
	* Called when an entity is detached, before it is actually detached.
	* iMuzzleIndex specifies the index of the muzzle from with it is detached from,
	* or -1 if it wasn't attached to a muzzle.
	*/
	event void OnWeaponDetachedAttachment(IEntity entity, int iMuzzleIndex);
}

/*!
\}
*/
