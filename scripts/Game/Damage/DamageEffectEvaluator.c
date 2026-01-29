class DamageEffectEvaluator : ScriptAndConfig
{
    //This is the DamageEffect evaluator.
    //Whenever ApplyEffect gets called on a DamageEffect, HandleEffectConsequences gets called
    //By writing in this class the consequences of effects getting applied, DamageEffects become more flexible.
    //E.g: Imagine that the consequence of a bullet effect is to add a bleeding effect. Vehicles shouldn't be getting bleeding effects.
    //    -If we write the logic of adding the bleeding on the DamageEffect, we need to check for the DamageManager type
    //    -If we write the logic on the DamageManager, we need to check for different DamageEffect types.
    //         If you only want the bleeding to apply to some characters, now we need to overcomplicate the logic inside of the damage Manager
    //         to handle those cases.
    //    -Therefore the solution is to apply the consequences through this evaluators. 
    //        The evaluator will handle the logic of DamageEffects being applied to the damage manager
    //        We can even change in runtime what evaluator is being used by a ExtendedDamageManager
    //
    //IMPORTANT: Whenever you create your own DamageEffect, you MUST create an overload of HandleEffectConsequences that uses your DamageEffect
	//inside of THIS class.
    //If the base class doesn't contain the overload, HandleEffectConsequences will not be called correctly for that DamageEffect type.
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(BaseDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_InstantDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_PersistentDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_BaseBulletDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_IncendiaryBulletDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_CollisionDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_ExplosionDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_BlastForceDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_IncendiaryDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_VehicleFireDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_FragmentationDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_MeleeDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_DrowningDamageEffect effect, ExtendedDamageManagerComponent dmgManager);
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_TourniquetDamageEffect effect, ExtendedDamageManagerComponent dmgManager);	
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_SalineDamageEffect effect, ExtendedDamageManagerComponent dmgManager);	
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_MorphineDamageEffect effect, ExtendedDamageManagerComponent dmgManager);
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_BandageDamageEffect effect, ExtendedDamageManagerComponent dmgManager);	
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_SpecialCollisionDamageEffect effect, ExtendedDamageManagerComponent dmgManager);
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_PersistentBarbedWireDamageEffect effect, ExtendedDamageManagerComponent dmgManager);
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_BarbedWireCuttingDamageEffect effect, ExtendedDamageManagerComponent dmgManager);
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_FallDamageEffect effect, ExtendedDamageManagerComponent dmgManager);	
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_AnimatedFallDamageEffect effect, ExtendedDamageManagerComponent dmgManager);
	
	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_RagdollFallDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_BaseBlastDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_ConcussionDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_BackBlastDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_MuzzleBlastDamageEffect effect, ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	void HandleEffectConsequences(SCR_CriticalExplosionDamageEffect effect, ExtendedDamageManagerComponent dmgManager);
}
