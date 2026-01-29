class SCR_BarbedWireCuttingDamageEffect : SCR_InstantDamageEffect
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, desc: "Multiplication of base barbed wire damageValue \n x = m/s", category: "Barbed wire damage")]
	protected float m_fDamageMultiplier;
	
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.EditBox, desc: "Chance that bleeding is started by the cut on the character \n 0 means no chance, 1 means always bleed", params: "0 1 0.001", category: "Barbed wire damage")]
	protected float m_fCauseBleedingChance;
	
	
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDamageMultiplier()
	{
		return m_fDamageMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBleedingChance()
	{
		return m_fCauseBleedingChance;
	}
}
