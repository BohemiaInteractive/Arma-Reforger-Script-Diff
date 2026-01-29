class SCR_PhysicalHitZonesRegenDamageEffect : SCR_DotDamageEffect
{
	protected float m_fDelayTimer = 0;
	protected float m_fLocalRegenDelayTime;
	protected float m_fDurationPerHitZone;
	protected float m_fAccurateTimeSlice;
	protected ref array<SCR_RegeneratingHitZone> m_aHitZonesToRegen = {};

	//ALWAYS OVERRIDE LIKE THIS. ALWAYS OVERRIDE THIS FUNCTION
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectAdded(dmgManager);
		SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!characterDmgManager)
			return;
		
		if (characterDmgManager.s_HealthSettings && !GetSkipRegenDelay(dmgManager))
			m_fLocalRegenDelayTime = characterDmgManager.s_HealthSettings.GetRegenerationDelay();
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		m_fDelayTimer += timeSlice;
		if (m_fDelayTimer < m_fLocalRegenDelayTime)
			return;
		
		if (m_aHitZonesToRegen.IsEmpty())
		{
			dmgManager.TerminateDamageEffect(this);
			return;
		}
		
		m_fAccurateTimeSlice = GetAccurateTimeSlice(timeSlice);
		m_fDurationPerHitZone = m_fAccurateTimeSlice/m_aHitZonesToRegen.Count();
		
		array<SCR_RegeneratingHitZone> hitZonesToRemove = {};
		SCR_RegeneratingHitZone hitZone;
		
		DotDamageEffectTimerToken token = UpdateTimer(m_fAccurateTimeSlice, dmgManager);
		
		for (int i = m_aHitZonesToRegen.Count() - 1; i > -1; i--)
		{
			hitZone = m_aHitZonesToRegen[i];
			if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
			{
				m_aHitZonesToRegen.Remove(i);
				continue;
			}
			
			DealCustomDot(hitZone, -hitZone.CalculatePassiveRegenDPS() * m_fAccurateTimeSlice, token, dmgManager);
		}
		
		//everything is fully healed
		if (m_aHitZonesToRegen.IsEmpty())
			dmgManager.TerminateDamageEffect(this);
	}

	//------------------------------------------------------------------------------------------------
	event override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);

		array<ref SCR_PersistentDamageEffect> bleedings = characterDmgManager.GetAllPersistentEffectsOfType(SCR_BleedingDamageEffect);
		if (!bleedings.IsEmpty())
			return true;

		array<ref SCR_PersistentDamageEffect> poisons = characterDmgManager.GetAllPersistentEffectsOfType(SCR_PoisonDamageEffect);
		if (!poisons.IsEmpty())
			return true;
		
		array<ref SCR_PersistentDamageEffect> persistentEffects = {};
		characterDmgManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_PhysicalHitZonesRegenDamageEffect, characterDmgManager.GetDefaultHitZone(), persistentEffects);
		
		//if the effect is already present on the hitzone, we dont add a second one.
		if (!persistentEffects.IsEmpty())
		{
			SCR_PhysicalHitZonesRegenDamageEffect actualEffect = SCR_PhysicalHitZonesRegenDamageEffect.Cast(persistentEffects[0]);

			// if regen already exists but owner got regen added again, reset hitZonesToHeal
			SetHitZonesToHeal(characterDmgManager);
			actualEffect.m_fDelayTimer = 0;
			return true;
		}
		
		// if no regen is present yet, add all physical hitzones to the HitZonesToHeal
		SetHitZonesToHeal(characterDmgManager, false);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetHitZonesToHeal(SCR_CharacterDamageManagerComponent characterDmgManager, bool clearHealedHZs = true)
	{
		m_aHitZonesToRegen.Clear();
		
		array<SCR_RegeneratingHitZone> hitZonesToAdd = {};
		characterDmgManager.GetRegeneratingHitZones(hitZonesToAdd);

		hitZonesToAdd.RemoveItem(characterDmgManager.GetBloodHitZone());
		hitZonesToAdd.RemoveItem(characterDmgManager.GetResilienceHitZone());

		foreach (SCR_RegeneratingHitZone hitZone : hitZonesToAdd)
		{
			if (clearHealedHZs && hitZone.GetDamageState() == EDamageState.UNDAMAGED)
				continue;
		
			m_aHitZonesToRegen.Insert(hitZone);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool DelayTimerRanOut()
	{
		return m_fDelayTimer > m_fLocalRegenDelayTime;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetSkipRegenDelay(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (characterDmgManager && characterDmgManager.s_HealthSettings)
			m_fLocalRegenDelayTime = characterDmgManager.s_HealthSettings.GetRegenerationDelay();
		
		return super.GetSkipRegenDelay(dmgManager);
	}
}
