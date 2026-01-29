class SCR_CharacterDamageManagerEvaluator : DamageEffectEvaluator
{
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_BaseBulletDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		CriticalThresholdTryCauseBleeding(effect, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_CollisionDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		CriticalThresholdTryCauseBleeding(effect, dmgManager);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_FragmentationDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		CriticalThresholdTryCauseBleeding(effect, dmgManager);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_MeleeDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);
	
		CriticalThresholdTryCauseBleeding(effect, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_CriticalExplosionDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);
	
		CriticalThresholdTryCauseBleeding(effect, dmgManager);
	}
		
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_BarbedWireCuttingDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);
		
		SCR_CharacterHitZone affectedHitZone = SCR_CharacterHitZone.Cast(effect.GetAffectedHitZone());
		if (!affectedHitZone)
			return;
		
		if (effect.GetBleedingChance() > Math.RandomFloat01())
			return; 
		
		EffectCauseBleeding(effect, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_BandageDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);

		array<HitZone> hitZonesOfGroup = {};
		damageMgr.GetHitZonesOfGroup(SCR_HitZone.Cast(effect.GetAffectedHitZone()).GetHitZoneGroup(), hitZonesOfGroup);

		array<ref SCR_PersistentDamageEffect> persistentEffects = damageMgr.GetAllPersistentEffectsOfType(SCR_BleedingDamageEffect);

		foreach (SCR_PersistentDamageEffect persistentEffect : persistentEffects)
		{
			if (!SCR_BleedingDamageEffect.Cast(persistentEffect))
				continue;

			foreach (HitZone hz : hitZonesOfGroup)
			{
				if (persistentEffect.GetAffectedHitZone() == hz)
				{
					damageMgr.TerminateDamageEffect(persistentEffect);
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_BaseBlastDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		super.HandleEffectConsequences(effect, dmgManager);

		if (effect.GetResilienceDamageValueMultiplier() == 0)
			return;

		SCR_CharacterDamageManagerComponent characterDmgMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!characterDmgMgr)
			return;

		SCR_CharacterResilienceHitZone resilienceHZ = characterDmgMgr.GetResilienceHitZone();
		if (!resilienceHZ)
			return;

		vector hitPosDirNorm[3];
		SCR_DamageContext context = new SCR_DamageContext(
			effect.GetDamageType(), 
			effect.GetTotalDamage() * effect.GetResilienceDamageValueMultiplier(), 
			hitPosDirNorm,
			characterDmgMgr.GetOwner(), 
			resilienceHZ, 
			effect.GetInstigator(),
			null,
			-1,
			-1);

		context.damageEffect = new SCR_ConcussionDamageEffect();
		characterDmgMgr.HandleDamage(context);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Asks the damagemanager to add a bleeding when conditions are met. Only instantDamageEffects can start bleedings
	//! \param[in] effect - Damage effect causing the bleeding. Must have a valid total damage amount and an affectedHitZone
	//! \param[in] dmgManager - ParentDamageManager which owns this evaluator
	private void EffectCauseBleeding(SCR_InstantDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterHitZone affectedHitZone = SCR_CharacterHitZone.Cast(effect.GetAffectedHitZone());
		if (!affectedHitZone)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!damageMgr)
			return;
		
		damageMgr.AddBleedingEffectOnHitZone(affectedHitZone);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Depending on the damage value and the criticalDamageThreshold, cause bleeding. 
	//! \param effect[in] - Damage effect causing the bleeding. Must have a valid total damage amount and an affectedHitZone
	//! \param dmgManager[in] - ParentDamageManager which owns this evaluator
	private void CriticalThresholdTryCauseBleeding(SCR_InstantDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterHitZone affectedHitZone = SCR_CharacterHitZone.Cast(effect.GetAffectedHitZone());
		if (!affectedHitZone)
			return;
		
		vector hitPosDirNorm[3];
		SCR_DamageContext context = new SCR_DamageContext(
			effect.GetDamageType(), 
			effect.GetTotalDamage(), 
			hitPosDirNorm,
			dmgManager.GetOwner(), 
			affectedHitZone, 
			effect.GetInstigator(),
			null,
			-1,
			-1);
		
		float var = affectedHitZone.ComputeEffectiveDamage(context, false);
		if (var < affectedHitZone.GetCriticalHealthThreshold())
			return; 
		
		EffectCauseBleeding(effect, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleEffectConsequences(SCR_PersistentBarbedWireDamageEffect effect, ExtendedDamageManagerComponent dmgManager)
	{
		if (!effect)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!damageMgr)
			return;

		SCR_BarbedWireCuttingDamageEffect consequence = effect.GetResultingEffect();
		if (!consequence)
			return;

		SCR_CharacterHitZone hitZone = damageMgr.GetRandomPhysicalHitZone();
		if (!hitZone)
			return;
		
		// Normalize damage across hitZone types
		float damage = hitZone.GetMaxHealth() * 0.01 * consequence.GetDamageMultiplier();
		
		vector hitPosDirNorm[3];
		SCR_DamageContext context = new SCR_DamageContext(EDamageType.MELEE, damage, hitPosDirNorm, dmgManager.GetOwner(), hitZone, Instigator.CreateInstigator(dmgManager.GetOwner()), null, -1, -1);
		
		context.damageEffect = consequence;
		
		dmgManager.HandleDamage(context);
	}
}
