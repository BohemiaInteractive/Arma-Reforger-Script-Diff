class SCR_HitZone : HitZone
{
	[Attribute(desc: "Rules for passing damage to parent, root and own default hitzone")]
	protected ref array<ref SCR_BaseDamagePassRule> m_aDamagePassRules;

	protected ref ScriptInvokerVoid m_OnHealthChanged;
	protected ref ScriptInvoker m_OnDamageStateChanged; // TODO: make it a ScriptInvoker w/ ScriptedHitZone
	
	//------------------------------------------------------------------------------------------------
	//! Hit zone group getter to be overridden
	//! \return Hit zone group
	EHitZoneGroup GetHitZoneGroup()
	{
		return EHitZoneGroup.VIRTUAL;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get event called when hitzone damage changes.
	//! \param createNew only create a script invoker if this is set to true
	//! \return Script invoker
	ScriptInvokerVoid GetOnHealthChanged(bool createNew = true)
	{
		if (!m_OnHealthChanged && createNew)
			m_OnHealthChanged = new ScriptInvokerVoid();

		return m_OnHealthChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get event called when hitzone damage state changes.
	//! \return Script invoker
	ScriptInvoker GetOnDamageStateChanged(bool createNew = true)
	{
		if (!m_OnDamageStateChanged && createNew)
			m_OnDamageStateChanged = new ScriptInvoker();

		return m_OnDamageStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Owner entity of the HitZone
	IEntity GetOwner()
	{
		HitZoneContainerComponent container = GetHitZoneContainer();
		if (container)
			return container.GetOwner();
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnHealthSet()
	{
		if (m_OnHealthChanged)
			m_OnHealthChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the damage state changes.
	override protected void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		if (m_OnDamageStateChanged)
			m_OnDamageStateChanged.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		if (damageContext.damageValue == 0)
			return 0;

		if (damageContext.damageType == EDamageType.TRUE || damageContext.damageType == EDamageType.PROCESSED_FRAGMENTATION)
			return damageContext.damageValue;

		//apply base multiplier
		float effectiveDamage = damageContext.damageValue * GetBaseDamageMultiplier();

		//apply damage multiplier for this specific damage type
		effectiveDamage *= GetDamageMultiplier(damageContext.damageType);

		//DOT doesn't get affected by damage reduction/thresholds, and neither does healing.
		if (isDOT || effectiveDamage < 0)
			return effectiveDamage;

		//apply flat damage reduction
		effectiveDamage -= GetDamageReduction();

		//if its less than the damage threshold we discard the damage.
		//if the damage to take becomes negative (healing) because of the flat damage reduction, this should reset it back to 0 dmg to take.
		if (effectiveDamage < GetDamageThreshold())
			effectiveDamage = 0;

		return effectiveDamage;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void DrawDebug();
#endif

	//------------------------------------------------------------------------------------------------
	/*!
	Pass damage based damage context and rules specified in hitzone configuration
	Receiving hitzones also have to inherit from SCR_HitZone
	//! param[in] damageContext - Damage context coming from HijackDamageHandling
	*/
	void ApplyDamagePassRules(notnull BaseDamageContext damageContext)
	{
		if (m_aDamagePassRules.IsEmpty())
			return;

		const SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());
		foreach (SCR_BaseDamagePassRule rule : m_aDamagePassRules)
		{
			rule.HandlePassedDamage(damageContext, this, damageManager);
		}
	}
}
