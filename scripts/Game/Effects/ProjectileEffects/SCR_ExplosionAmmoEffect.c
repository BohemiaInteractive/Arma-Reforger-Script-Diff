void OnExplosionAmmoEffectMethod(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed);
typedef func OnExplosionAmmoEffectMethod;
typedef ScriptInvokerBase<OnExplosionAmmoEffectMethod> OnExplosionAmmoEffectInvoker;

class SCR_ExplosionAmmoEffect : BaseProjectileEffect
{	
	protected static ref OnExplosionAmmoEffectInvoker s_OnExplosionAmmoEffect;
	
	//------------------------------------------------------------------------------------------------
	//! Get event called when the effect is being applied for explosion ammo.
	//! \return OnExplosionAmmoEffectInvoker
	static OnExplosionAmmoEffectInvoker GetOnExplosionAmmoEffect()
	{
		if (!s_OnExplosionAmmoEffect)
			s_OnExplosionAmmoEffect = new OnExplosionAmmoEffectInvoker();

		return s_OnExplosionAmmoEffect;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEffect(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed)
	{	
		if (s_OnExplosionAmmoEffect)
			s_OnExplosionAmmoEffect.Invoke(pHitEntity, outMat, damageSource, instigator, colliderName, speed);
	}
}