class SCR_PassiveHitZoneRegenDamageEffect : SCR_DotDamageEffect
{
	protected float m_fDelayTimer = 0;
	protected float m_fLocalRegenDelayTime;
	protected SCR_RegeneratingHitZone m_HitZoneToRegen;

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
		
		m_HitZoneToRegen = SCR_RegeneratingHitZone.Cast(GetAffectedHitZone());
		if (!m_HitZoneToRegen)
			Terminate();
		
		SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (characterDmgManager && characterDmgManager.s_HealthSettings && !GetSkipRegenDelay(dmgManager))
			m_fLocalRegenDelayTime = characterDmgManager.s_HealthSettings.GetRegenerationDelay();
		
		if (GetDPS() == -1)
			SetDPS(-m_HitZoneToRegen.CalculatePassiveRegenDPS());
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		m_fDelayTimer += timeSlice;

		if (m_fDelayTimer < m_fLocalRegenDelayTime)
			return;

		DealDot(timeSlice, dmgManager);

		//everything is fully healed
		if (m_HitZoneToRegen.GetHealthScaled() == 1)
			dmgManager.TerminateDamageEffect(this);
	}
}
