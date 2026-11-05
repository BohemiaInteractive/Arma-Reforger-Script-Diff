// Persistent marker that increases positive stamina regeneration while it is present.
// The effect does not modify a hit zone, its lifetime is maintained by the timer.
class SCR_FlaskStaminaRegenBoostDamageEffect : SCR_DotDamageEffect
{
	[Attribute("0.5", UIWidgets.EditBox, "Additional stamina regeneration in percentage, 1 means the regeneration is doubled.")]
	protected float m_fRegenerationBonus;

	[Attribute(defvalue: "0.7", desc: "Percentage of the stamina drain that will be mitigated when this damage effect is present on a character", params: "0 1 0.01")]
	protected float m_fStaminaDrainReduction;

	protected bool m_bReplicatedFiniteDuration;
	protected float m_fReplicatedRemainingDuration;
	protected WorldTimestamp m_ReplicatedDurationTimestamp;

	//------------------------------------------------------------------------------------------------
	float GetRegenerationBonus()
	{
		return Math.Max(m_fRegenerationBonus, 0);
	}

	//------------------------------------------------------------------------------------------------
	float GetStaminaDrainReduction()
	{
		return m_fStaminaDrainReduction;
	}

	//------------------------------------------------------------------------------------------------
	bool HasFiniteDuration()
	{
		return GetMaxDuration() > 0 || m_bReplicatedFiniteDuration;
	}

	//------------------------------------------------------------------------------------------------
	float GetRemainingDuration()
	{
		float maximumDuration = GetMaxDuration();
		if (GetMaxDuration() > 0)
			return Math.Max(maximumDuration - GetCurrentDuration(), 0);

		if (!m_bReplicatedFiniteDuration)
			return 0;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world || m_ReplicatedDurationTimestamp == 0)
			return m_fReplicatedRemainingDuration;

		float elapsedSeconds = Math.Max(world.GetServerTimestamp().DiffMilliseconds(m_ReplicatedDurationTimestamp), 0) * 0.001;
		return Math.Max(m_fReplicatedRemainingDuration - elapsedSeconds, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetReplicatedDurationSnapshot(float remainingDuration)
	{
		remainingDuration = Math.Max(remainingDuration, 0);
		m_bReplicatedFiniteDuration = remainingDuration > 0;
		m_fReplicatedRemainingDuration = remainingDuration;
		m_ReplicatedDurationTimestamp = null;

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			m_ReplicatedDurationTimestamp = world.GetServerTimestamp();
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectAdded(dmgManager);

		IEntity character = dmgManager.GetOwner();
		if (!character)
			return;

		SCR_CharacterStaminaComponent staminaComponent = SCR_CharacterStaminaComponent.Cast(character.FindComponent(SCR_CharacterStaminaComponent));
		if (staminaComponent)
			staminaComponent.StartStaminaRegenerationBoostTracking(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectRemoved(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectRemoved(dmgManager);

		IEntity character = dmgManager.GetOwner();
		if (!character)
			return;

		SCR_CharacterStaminaComponent staminaComponent = SCR_CharacterStaminaComponent.Cast(character.FindComponent(SCR_CharacterStaminaComponent));
		if (staminaComponent)
			staminaComponent.StopStaminaRegenerationBoostTracking(this);
	}

	//------------------------------------------------------------------------------------------------
	override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_FlaskStaminaRegenBoostDamageEffect boostEffect = SCR_FlaskStaminaRegenBoostDamageEffect.Cast(dmgManager.FindDamageEffectOfType(SCR_FlaskStaminaRegenBoostDamageEffect));
		if (!boostEffect)
			return false;

		float refreshDuration = GetMaxDuration();
		if (refreshDuration > 0)
			boostEffect.SetMaxDuration(boostEffect.GetCurrentDuration() + refreshDuration);
		else
			boostEffect.SetMaxDuration(0);

		boostEffect.ApplyEffect(dmgManager);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);
		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		if (IsProxy())
			return;

		UpdateTimer(timeSlice, dmgManager);
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

		w.WriteFloat(m_fRegenerationBonus);
		w.WriteFloat(m_fStaminaDrainReduction);
		w.WriteFloat(GetRemainingDuration());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Load(ScriptBitReader r)
	{
		if (!super.Load(r))
			return false;

		r.ReadFloat(m_fRegenerationBonus);
		r.ReadFloat(m_fStaminaDrainReduction);
		float remainingDuration;
		r.ReadFloat(remainingDuration);
		SetReplicatedDurationSnapshot(remainingDuration);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SaveApplyEffect(ScriptBitWriter w)
	{
		float remainingDuration = 0;
		if (GetMaxDuration() > 0)
			remainingDuration = Math.Max(GetMaxDuration() - GetCurrentDuration(), 0);

		w.WriteFloat(remainingDuration);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool LoadApplyEffect(ScriptBitReader r)
	{
		float remainingDuration;
		r.ReadFloat(remainingDuration);
		SetReplicatedDurationSnapshot(remainingDuration);
		SetMaxDuration(Math.Max(remainingDuration, 0));
		return true;
	}
}
