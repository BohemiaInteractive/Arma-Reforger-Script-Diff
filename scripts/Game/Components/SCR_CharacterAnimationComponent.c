class SCR_CharacterAnimationComponentClass: CharacterAnimationComponentClass
{
}

class SCR_CharacterAnimationComponent: CharacterAnimationComponent
{
			
	[Attribute(defvalue: "1000", uiwidget: UIWidgets.EditBox, desc: "Multiplies impulse of blastForceDamageEffect on explosives\nActual impulse = impulse * Additive Explosion Impulse Multiplier", params: "0 inf 0.001",)]
	protected float m_fExplosionImpulseMultiplier;
	
	//-----------------------------------------------------------------------------------------------------------
	//! Using AddRagdollEffectorDamage calls, you can specify parameters of the effector that affects ragdoll.
	//! Return true if the damage has been handled, otherwise, it will use default gamecode implementation.
	override bool HandleDamage(BaseDamageContext damageContext, IEntity owner)
	{
		if (m_fExplosionImpulseMultiplier == 1)
			return false;
		
		// Use default impulse behavior for melee and kinetic damage
		if (damageContext.damageType == EDamageType.MELEE || damageContext.damageType == EDamageType.KINETIC)
			return false;
		
		// use default impulse behavior for collisions, unless the collision is caused by animatedFallDamage
		// then return true and add no RagdollEffectorDamage, since it's already handled in HandleAnimatedFallDamage() in SCR_CharacterDamageManagerComponent
		if (damageContext.damageType == EDamageType.COLLISION)
		{
			if (SCR_AnimatedFallDamageEffect.Cast(damageContext.damageEffect))
				return true;
			
			return false;
		}

		// If damageContext contains a blastForceDamageEffect, apply m_fExplosionImpulseMultiplier
		if (SCR_BlastForceDamageEffect.Cast(damageContext.damageEffect))
			AddRagdollEffectorDamage(owner.CoordToLocal(damageContext.hitPosition), owner.VectorToLocal(damageContext.hitDirection), damageContext.damageValue * m_fExplosionImpulseMultiplier, 0.3, 2);
		
		return true;
	}
}