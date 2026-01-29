class SCR_BaseBlastDamageEffect : SCR_InstantDamageEffect
{
	[Attribute("1", desc: "Final amount of damage that was dealt to the character will be multiplied by this factor and dealt as additional damage to the character's resilience.\nIn order for the character to become unconscious, he has to take about 70 resilience damage in a relatively short amount of time due to the passive regeneration.", params: "0 inf")]
	protected float m_fResilienceDamageValueMultiplier;

	//------------------------------------------------------------------------------------------------
	float GetResilienceDamageValueMultiplier()
	{
		return m_fResilienceDamageValueMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}
}
