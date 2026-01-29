class SCR_HitZone : HitZone
{
	[Attribute(desc: "Rules for passing damage to parent, root and own default hitzone")]
	protected ref array<ref SCR_DamagePassRule> m_aDamagePassRules;

	[Attribute(defvalue: "1", desc: "Fire damage multiplier\n[x * 100%]", params: "0 1000 0.01")]
	protected float m_fFireMultiplier;

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

		SCR_HitZone defaultHitZone;
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());
		if (damageManager)
		{
			defaultHitZone = SCR_HitZone.Cast(damageManager.GetDefaultHitZone());

			if (defaultHitZone == this)
				defaultHitZone = null;
		}

		SCR_HitZone rootHitZone;
		IEntity owner = GetOwner();
		IEntity root = owner.GetRootParent();
		if (root)
		{
			damageManager = SCR_DamageManagerComponent.GetDamageManager(root);
			if (damageManager)
				rootHitZone = SCR_HitZone.Cast(damageManager.GetDefaultHitZone());

			if (rootHitZone == this)
				rootHitZone = null;
		}

		SCR_HitZone parentHitZone;
		IEntity parent = owner.GetParent();
		if (parent)
		{
			damageManager = SCR_DamageManagerComponent.GetDamageManager(parent);
			if (damageManager)
				parentHitZone = SCR_HitZone.Cast(damageManager.GetDefaultHitZone());

			if (parentHitZone == this)
				parentHitZone = null;
		}

		// Forward non-DOT damage to root or parent default hitzone, ignoring own base damage multiplier
		BaseDamageContext context;
		foreach (SCR_DamagePassRule rule : m_aDamagePassRules)
		{
			// If damage types are defined, only allow passing specified damage types
			if (!rule.m_aSourceDamageTypes.IsEmpty() && !rule.m_aSourceDamageTypes.Contains(damageContext.damageType))
				continue;

			// If damage states are defined, only allow passing while damage state is allowed
			if (!rule.m_aDamageStates.IsEmpty() && !rule.m_aDamageStates.Contains(GetDamageState()))
				continue;

			context = BaseDamageContext.Cast(damageContext.Clone());
			context.damageValue *= rule.m_fMultiplier;

			// Change passed damage type unless it is set to true
			if (rule.m_eOutputDamageType != EDamageType.TRUE)
				context.damageType = rule.m_eOutputDamageType;

			if (rule.m_bPassToRoot && rootHitZone)
				rootHitZone.HandlePassedDamage(context);

			if (rule.m_bPassToParent && parentHitZone)
				parentHitZone.HandlePassedDamage(context);

			if (rule.m_bPassToDefaultHitZone && defaultHitZone)
				defaultHitZone.HandlePassedDamage(context);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Handle damage in this hit zone. Clones the damage context so that it does not override passed one
	//! \param[in] damageContext - damage context to be received in this hit zone
	*/
	void HandlePassedDamage(notnull BaseDamageContext damageContext)
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(GetHitZoneContainer());
		if (!damageManager)
			return;

		BaseDamageContext context = BaseDamageContext.Cast(damageContext.Clone());
		context.struckHitZone = this;

		damageManager.HandleDamage(context);
	}
}
