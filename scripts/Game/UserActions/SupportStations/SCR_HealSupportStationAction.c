class SCR_HealSupportStationAction : SCR_BaseDamageHealSupportStationAction
{
	[Attribute("#AR-SupportStation_Heal_ActionInvalid_FullHealth", desc: "Text shown on action if undamaged", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidDamaged;

	[Attribute("#AR-Inventory_Bleeding", desc: "Text shown on action if character is bleeding and action cannot be performed", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidCharacterIsBleeding;

	//[Attribute("loc Can not heal more (Disabled)", desc: "Character cannot heal more as max healing is reached", uiwidget: UIWidgets.LocaleEditBox)]
	//protected LocalizedString m_sInvalidMaxHealingReached;

	//[Attribute("loc Seek medical base (Disabled)", desc: "Character cannot heal more as max healing is reached for healing with field service (Ambulance).", uiwidget: UIWidgets.LocaleEditBox)]
	//protected LocalizedString m_sInvalidMaxHealingReachedField;

	[Attribute("#AR-SupportStation_Heal_ActionInvalid_Needs_SupportStation_Emergency", desc: "Character cannot heal more as max healing is reached for healing with emergency heal (No medical services in the area)", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidMaxHealingReachedEmergency;

	[Attribute(ECharacterHitZoneGroup.VIRTUAL.ToString(), UIWidgets.ComboBox, "Which hitzone group will be healed, VIRTUAL means all hitzones are healed", enumType:ECharacterHitZoneGroup)]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;

	[Attribute("0", desc: "If this heal action will also heal the blood hitzone")]
	protected bool m_bHealBloodHitzone;

	protected HitZone m_BloodHitZone;

	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.HEAL;
	}

	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RequiresGadget()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override int GetActionDecimalCount()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		//~ Entity is undamaged
		if (reasonInvalid == ESupportStationReasonInvalid.HEAL_ENTITY_UNDAMAGED)
			return m_sInvalidDamaged;
		else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_CHARACTER_IS_BLEEDING)
			return m_sInvalidCharacterIsBleeding;
		//else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED)
		//	return m_sInvalidMaxHealingReached;
		//else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD)
		//	return m_sInvalidMaxHealingReachedField;
		else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY)
			return m_sInvalidMaxHealingReachedEmergency;

		return super.GetInvalidPerformReasonString(reasonInvalid);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DamageManagerComponent || m_aHitZonesToHeal.IsEmpty())
			return false;

		foreach (HitZone hitzone : m_aHitZonesToHeal)
		{
			//~ Has a hitzone that is damaged so show
			if (hitzone.GetHealthScaled() != 1)
				return super.CanBeShownScript(user);
		}

		//~ Not damaged
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		SCR_CharacterDamageManagerComponent charDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_DamageManagerComponent);
		if (!charDamageManager)
			return super.CanBePerformedScript(user);

		//~ Can heal blood hitzone but somewhere the character is bleeding. Note: m_BloodHitZone will be null if m_bHealBloodHitzone is false
		if (m_BloodHitZone && charDamageManager.IsBleeding())
		{
			SetCanPerform(false, ESupportStationReasonInvalid.HEAL_CHARACTER_IS_BLEEDING);
			return false;
		}

		//~ Check if the character is bleeding at the hitzones this action heals
		foreach (HitZone hitZone : m_aHitZonesToHeal)
		{
			SCR_RegeneratingHitZone regenHitZone = SCR_RegeneratingHitZone.Cast(hitZone);
			if (!regenHitZone)
				continue;

			if (regenHitZone.GetHitZoneDamageOverTime(EDamageType.BLEEDING) > 0)
			{
				SetCanPerform(false, ESupportStationReasonInvalid.HEAL_CHARACTER_IS_BLEEDING);
				return false;
			}
		}

		return super.CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	//! return Return the Blood hitzone to heal if any is set
	HitZone GetBloodHitZoneToHeal()
	{
		return m_BloodHitZone;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsStabilizedReason()
	{
		return m_eCannotPerformReason == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY;
	}
	
	//------------------------------------------------------------------------------------------------
	//! return the Health code color for nearby interactions
	override SCR_ENearbyInteractionContextColors GetHealthStatus()
	{
		int healthState = m_DamageIntensityHolder.GetValidIntensityType(m_aHitZonesToHeal);
		
		if (healthState == SCR_EDamageIntensityType.NO_DAMAGE)
			return SCR_ENearbyInteractionContextColors.DEFAULT;
		
		if (IsInProgress())
			return SCR_ENearbyInteractionContextColors.HEALING;
		
		if (IsStabilizedReason())
			return SCR_ENearbyInteractionContextColors.STABLE;
		
		if (healthState < SCR_EDamageIntensityType.HIGH)
			return SCR_ENearbyInteractionContextColors.STABLE;
		
		return SCR_ENearbyInteractionContextColors.SEVERE;
	}

	//------------------------------------------------------------------------------------------------
	protected override void SetHitZonesToHeal()
	{
		m_aHitZonesToHeal.Clear();

		//~ If it should heal the blood hitzone then set reference to it
		if (m_bHealBloodHitzone)
		{
			SCR_CharacterDamageManagerComponent charDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_DamageManagerComponent);
			if (charDamageManager)
				m_BloodHitZone = charDamageManager.GetBloodHitZone();
		}

		//~ Virtual gets all hitzones
		if (m_eHitZoneGroup == ECharacterHitZoneGroup.VIRTUAL)
		{
			super.SetHitZonesToHeal();
			return;
		}

		//~ Get specific hitzone group
		m_DamageManagerComponent.GetHitZonesOfGroup(m_eHitZoneGroup, m_aHitZonesToHeal);
	}
}
