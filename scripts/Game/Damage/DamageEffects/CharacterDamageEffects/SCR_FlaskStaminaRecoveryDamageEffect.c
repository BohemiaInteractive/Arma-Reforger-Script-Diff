// One-shot stamina and hydration recovery applied after the drinking animation completes.
class SCR_FlaskStaminaRecoveryDamageEffect : SCR_InstantDamageEffect
{
	[Attribute("0.25", UIWidgets.EditBox, "Immediate stamina recovery. Stamina uses the 0..1 range.")]
	protected float m_fStaminaRecovery;

	[Attribute("1", UIWidgets.EditBox, "Immediate hydration recovery. Hydration uses the 0..1 range.")]
	protected float m_fHydrationRecovery;

	//------------------------------------------------------------------------------------------------
	float GetStaminaRecovery()
	{
		return Math.Max(m_fStaminaRecovery, 0);
	}

	//------------------------------------------------------------------------------------------------
	float GetHydrationRecovery()
	{
		return Math.Max(m_fHydrationRecovery, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);
		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override EDamageType GetDefaultDamageType()
	{
		return EDamageType.REGENERATION;
	}

	//------------------------------------------------------------------------------------------------
	override bool Save(ScriptBitWriter w)
	{
		if (!super.Save(w))
			return false;

		w.WriteFloat(m_fStaminaRecovery);
		w.WriteFloat(m_fHydrationRecovery);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Load(ScriptBitReader r)
	{
		if (!super.Load(r))
			return false;

		r.ReadFloat(m_fStaminaRecovery);
		r.ReadFloat(m_fHydrationRecovery);
		return true;
	}
}
