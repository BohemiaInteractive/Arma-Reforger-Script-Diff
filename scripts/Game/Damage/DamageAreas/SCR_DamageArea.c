class SCR_DamageArea : DamageArea
{
	[Attribute(defvalue: "1", desc: "Should this area be automatically registered to the system, when owner is initalized")]
	protected bool m_bRegisterAutomatically;

	[Attribute(desc: "Damage type of the damage which is going to be applied, when player enters the area.\nWhen none is selected the damage is no applied.", uiwidget: UIWidgets.ComboBox, enumType: EDamageType)]
	protected EDamageType m_eDamageType;

	[Attribute(desc: "How much damage should be dealt upon entering the zone")]
	protected float m_fDamageValue;

	[Attribute()]
	protected ref BaseDamageEffect m_DamageEffect;

	[Attribute(defvalue: "0", desc: "Remove Effect When Leaving The Area\nNOTE:If component has many areas with same Damage Effect, then leaving one zone will cause a removal of all instances of this Damage Effects type, that was applied by this component owner.")]
	protected bool m_bRemoveEffectWhenLeavingTheArea;

	[Attribute(defvalue: SCR_EHitZoneSelectionMode.DEFAULT.ToString(), desc: "Determines how hit zone is selected.\nThis attribute is also passed to the Custom Hit Zone Selector when such is present.", uiwidget: UIWidgets.ComboBox, enumType: SCR_EHitZoneSelectionMode)]
	protected SCR_EHitZoneSelectionMode m_eHitZoneSelectionMode;

	[Attribute(desc: "Custom selector of the hit zones. If not provided then basic area logic is used.")]
	protected ref SCR_BaseHitZoneSelector m_CustomHitZoneSelector;

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsRegisteredAutomatically()
	{
		return m_bRegisterAutomatically;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when an entity enters this DamageArea this frame
	//! \param[in] entity
	override void OnAreaEntered(notnull IEntity entity)
	{
		const SCR_ExtendedDamageManagerComponent dmgMgr = SCR_ExtendedDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(entity));
		if (!dmgMgr)
			return;

		HitZone hitZone = GetAffectedHitZone(dmgMgr, m_eHitZoneSelectionMode);
		if (!hitZone)
			return;

		if (m_fDamageValue == 0 || m_eDamageType == int.MAX) //int.MAX is equal to selecting in the prefab the option 'none' for m_eDamageType
		{
			dmgMgr.AddDamageEffect(GetDamageEffect(dmgMgr, hitZone));
			return;
		}

		const IEntity owner = GetParent();
		vector posDirNorm[3];
		posDirNorm[0] = entity.GetOrigin();
		SCR_DamageContext context = new SCR_DamageContext(m_eDamageType, m_fDamageValue, posDirNorm,
														entity, hitZone, Instigator.CreateInstigator(owner),
														null, 0, 0);

		context.damageEffect = GetDamageEffect(dmgMgr, hitZone);
		context.damageSource = owner;
		dmgMgr.HandleDamage(context);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when an entity exits this DamageArea this frame
	//! \param[in] entity
	override void OnAreaExit(IEntity entity)
	{
		if (!entity)
			return;

		if (!m_bRemoveEffectWhenLeavingTheArea)
			return;

		const SCR_ExtendedDamageManagerComponent dmgMgr = SCR_ExtendedDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(entity));
		if (!dmgMgr)
			return;

		array<ref SCR_PersistentDamageEffect> damageEffects = {};
		if (dmgMgr.FindAllDamageEffectsOfType(GetDamageEffect().Type(), damageEffects) < 1)
			return;

		const IEntity owner = GetParent();
		foreach (SCR_PersistentDamageEffect effect : damageEffects)
		{
			if (effect.GetInstigator().GetInstigatorEntity() == owner)
				effect.Terminate();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] dmgMgr damage manager of the entity which is going to receive this damage effect
	//! \param[in] affectedHitZone hit zone to which this damage effect should be applied
	//! \returns Damage effect which is applied by this area
	protected BaseDamageEffect GetDamageEffect(SCR_DamageManagerComponent dmgMgr = null, HitZone affectedHitZone = null)
	{
		return m_DamageEffect;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] dmgMgr damage manager of the entity which is going to receive this damage effect
	//! \param[in] affectedHitZone hit zone to which this damage effect should be applied
	//! \returns Damage effect which is applied by this area
	protected HitZone GetAffectedHitZone(notnull SCR_DamageManagerComponent dmgMgr, SCR_EHitZoneSelectionMode hitZoneSelectionMode)
	{
		if (m_CustomHitZoneSelector)
		{
			HitZone hitZone = m_CustomHitZoneSelector.SelectHitZone(dmgMgr, hitZoneSelectionMode);
			if (hitZone)
				return hitZone;

			if (m_CustomHitZoneSelector.ShouldAbortWhenNoCompatibleHitZonesWereFound())
				return null;
		}

		if (hitZoneSelectionMode == SCR_EHitZoneSelectionMode.DEFAULT)
			return dmgMgr.GetDefaultHitZone();

		array<HitZone> hitZones = {};
		if (hitZoneSelectionMode == SCR_EHitZoneSelectionMode.RANDOM)
			dmgMgr.GetAllHitZones(hitZones);
		else
			dmgMgr.GetPhysicalHitZones(hitZones);

		if (hitZones.IsEmpty())
			return null;

		if (hitZoneSelectionMode == SCR_EHitZoneSelectionMode.FIRST_PHYSICAL)
			return hitZones[0];

		return hitZones.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayInfo()
	{
		DbgUI.Text("Area information");
		DbgUI.Text("Area prefab = "+SCR_DamageAreaComponent.GetPrefabname(GetParent()));
	}
}
