class SCR_DotDamageArea : SCR_DamageArea
{
	[Attribute(desc: "How much damage should be dealt over time of being in the area.")]
	protected float m_fDotDamage;

	[Attribute(defvalue: "0", desc: "For how long should the effect last.\n0 means infinte, unless entity leaves the area and RemoveEffectWhenLeavingTheArea is true.", params: "0 inf 0.01")]
	protected float m_fEffectDuration;

	[Attribute(defvalue: "1", desc: "If m_bRemoveEffectWhenLeavingTheArea is false and this is true then effect will last as long as the character is in the area, and when he exits the area, then the effect will linger for the amount of time specified in m_fEffectDuration.\nNOTE: Same limitation as with m_bRemoveEffectWhenLeavingTheArea apply here as well.")]
	protected bool m_bAddDurationOnExit;

	[Attribute(defvalue: "0", desc: "Dot Damage Type", uiwidget: UIWidgets.ComboBox, enumType: EDamageType)]
	protected EDamageType m_eDotDamageType;

	//------------------------------------------------------------------------------------------------
	override protected BaseDamageEffect GetDamageEffect(SCR_DamageManagerComponent dmgMgr = null, HitZone affectedHitZone = null)
	{
		if (!dmgMgr && !affectedHitZone)
			return m_DamageEffect;

		SCR_DotDamageEffect output = SCR_DotDamageEffect.Cast(m_DamageEffect.Clone());
		output.SetDPS(m_fDotDamage);
		output.SetDamageType(m_eDotDamageType);
		output.SetAffectedHitZone(affectedHitZone);
		output.SetInstigator(Instigator.CreateInstigator(GetParent()));
		if (!m_bAddDurationOnExit)
			output.SetMaxDuration(m_fEffectDuration);

		return output;
	}

	//------------------------------------------------------------------------------------------------
	override void OnAreaExit(IEntity entity)
	{
		if (m_bRemoveEffectWhenLeavingTheArea)
		{
			super.OnAreaExit(entity);
			return;
		}

		if (!m_bAddDurationOnExit)
			return;

		if (!entity)
			return;

		const SCR_ExtendedDamageManagerComponent dmgMgr = SCR_ExtendedDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(entity));
		if (!dmgMgr)
			return;

		array<ref SCR_PersistentDamageEffect> damageEffects = {};
		if (dmgMgr.FindAllDamageEffectsOfType(GetDamageEffect().Type(), damageEffects) < 1)
			return;

		const IEntity owner = GetParent();
		SCR_DotDamageEffect dotEffect;
		foreach (SCR_PersistentDamageEffect effect : damageEffects)
		{
			dotEffect = SCR_DotDamageEffect.Cast(effect);
			if (!dotEffect)
				continue;

			if (dotEffect.GetInstigator().GetInstigatorEntity() == owner)
				dotEffect.SetMaxDuration(dotEffect.GetCurrentDuration() + m_fEffectDuration);
		}
	}
}