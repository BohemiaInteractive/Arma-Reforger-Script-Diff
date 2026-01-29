class SCR_RepairAtSupportStationAction : SCR_BaseDamageHealSupportStationAction
{
	[Attribute("#AR-SupportStation_Repair_ActionInvalid_NoDamage", desc: "Text shown on action if undamaged", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidDamaged;

	[Attribute("#AR-SupportStation_Repair_ActionInvalid_Needs_SupportStation_Field", desc: "Vehicle cannot be repaired any more as max healing is reached", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidMaxHealingReached_Field;

	[Attribute("#AR-SupportStation_Repair_ActionInvalid_Needs_SupportStation_Emergency", desc: "Vehicle cannot be repaired any more as max healing is reached", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidMaxHealingReached_Emergency;

	[Attribute("#AR-ActionInvalid_OnFire", desc: "Text shown on action if entity is on fire", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidOnFire;

	[Attribute("#AR-Editor_ContextAction_Extinguish_Name", desc: "Action name when entity is on fire. Only when hitzones are FlammableHitZones", uiwidget: UIWidgets.LocaleEditBox, category: "Heal/Repair Support Station")]
	protected LocalizedString m_sOnFireOverrideActionName;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Which hitzone groups will be healed, Leave empty if all need to be healed. (VIRTUAL is invalid and will be skipped). The first entry will be used for notifications", enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected ref array<EVehicleHitZoneGroup> m_aHitZoneGroups;

	[Attribute(desc: "Used to get the group name of which is going to be repaired")]
	protected ref SCR_HitZoneGroupNameHolder m_HitZoneGroupNames;

	[Attribute(desc: "Any addition Hitzone ID. This is generic and used for example to get the correct fuel tank using it as an ID")]
	protected ref array<int> m_aHitZoneAdditionalIDs;

	[Attribute("0", desc: "For some repair actions you want the system to know what it will be repairing, eg: Wheels, but the wheels have no hitzones with wheel hitzone group so just get all the hitzones on the entity. But the action and notifications still know they system is repairing the wheels")]
	protected bool m_bAlwaysRepairAllHitZones;

	[Attribute("1", desc: "if true then the action will not be shown if the entity (or root parent entity) is on fire")]
	protected bool m_bHideIfEntityOnFire;

	protected SCR_DamageManagerComponent m_OnFireCheckDamageManager;

	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.REPAIR;
	}

	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		//~ Entity is undamaged
		if (reasonInvalid == ESupportStationReasonInvalid.HEAL_ENTITY_UNDAMAGED)
			return m_sInvalidDamaged;
		else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_ENTITY_ONFIRE)
			return m_sInvalidOnFire;
		else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD)
			return m_sInvalidMaxHealingReached_Field;
		else if (reasonInvalid == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY)
			return m_sInvalidMaxHealingReached_Emergency;

		return super.GetInvalidPerformReasonString(reasonInvalid);
	}

	//------------------------------------------------------------------------------------------------
	int GetHitZoneGroups(out notnull array<EVehicleHitZoneGroup> hitZoneGroup)
	{
		hitZoneGroup.Copy(m_aHitZoneGroups);
		return hitZoneGroup.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RequiresGadget()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DamageManagerComponent || m_aHitZonesToHeal.IsEmpty())
			return false;

		if (!m_DamageManagerComponent.CanBeHealed(false))
			return false;

		// Only show damaged entries
		if (m_DamageIntensityHolder.GetValidIntensityType(m_aHitZonesToHeal) < SCR_EDamageIntensityType.LOW)
			return false;

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool CanShowDestroyed()
	{
		//~ If HULL is not included then it can always be repaired
		if (!m_aHitZoneGroups.Contains(EVehicleHitZoneGroup.HULL))
			return true;

		//~ Don't show if hull is destrpued
		return super.CanShowDestroyed();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		//~ If hitzones to heal are not on fire but the entity is then hide make the action invalid
		if (m_OnFireCheckDamageManager && m_OnFireCheckDamageManager.IsDamagedOverTime(EDamageType.FIRE) && !m_OnFireCheckDamageManager.IsOnFire(m_aHitZonesToHeal))
		{
			SetCanPerform(false, ESupportStationReasonInvalid.HEAL_ENTITY_ONFIRE);

			//~ Resets the support station so override action name is removed
			m_SupportStationComponent = null;
			return false;
		}

		if (!m_DamageManagerComponent.CanBeHealed(false))
		{
			SetCanPerform(false, ESupportStationReasonInvalid.HEAL_ENTITY_UNDAMAGED);

			//~ Resets the support station so override action name is removed
			m_SupportStationComponent = null;
			return false;
		}

		return super.CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	protected override void SetHitZonesToHeal()
	{
		//~ Virtual gets all hitzones. Or if wheels do the same as wheels have no additional hitzones
		if (m_aHitZoneGroups.IsEmpty() || m_bAlwaysRepairAllHitZones)
		{
			super.SetHitZonesToHeal();
			return;
		}

		m_aHitZonesToHeal.Clear();

		//~ Get the correct fuel tank to repair
		if (!m_aHitZoneAdditionalIDs.IsEmpty() && m_aHitZoneGroups.Contains(EVehicleHitZoneGroup.FUEL_TANKS))
		{
			SCR_FuelHitZone fuelHitZone;
			SCR_HitZone scrHitZone;

			array<HitZone> allHitZones = {};

			m_DamageManagerComponent.GetAllHitZonesInHierarchy(allHitZones);

			foreach (HitZone hitZone : allHitZones)
			{
				scrHitZone = SCR_HitZone.Cast(hitZone);

				if (scrHitZone.GetHitZoneGroup() != EVehicleHitZoneGroup.FUEL_TANKS)
					continue;

				//~ Not a fuel hitzone but part of the group
				fuelHitZone = SCR_FuelHitZone.Cast(hitZone);
				if (!fuelHitZone)
					m_aHitZonesToHeal.Insert(hitZone);

				if (!m_aHitZoneAdditionalIDs.Contains(fuelHitZone.GetFuelTankID()))
					continue;

				//~ Insert Fuel tank
				m_aHitZonesToHeal.Insert(hitZone);
			}
		}

		//~ Get hitzone groups (Fuel tanks are already obtained)
		foreach (EVehicleHitZoneGroup hitZoneGroup : m_aHitZoneGroups)
		{
			if (hitZoneGroup == EVehicleHitZoneGroup.VIRTUAL || (!m_aHitZoneAdditionalIDs.IsEmpty() && hitZoneGroup == EVehicleHitZoneGroup.FUEL_TANKS))
				continue;

			m_DamageManagerComponent.GetHitZonesOfGroup(hitZoneGroup, m_aHitZonesToHeal, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{
		//~ Don't show damage state when on fire
		if (m_OnFireCheckDamageManager.IsOnFire(m_aHitZonesToHeal))
			return string.Empty;

		return super.GetActionStringParam();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_HitZoneGroupNames)
			return super.GetActionNameScript(outName);

		//~ Set extinguish fire action name
		if (m_OnFireCheckDamageManager.IsOnFire(m_aHitZonesToHeal))
			outName = m_sOnFireOverrideActionName;
		//~ Override action name
		else if (m_SupportStationComponent)
			outName = m_SupportStationComponent.GetOverrideUserActionName();

		//~ Default action name
		if (outName.IsEmpty())
		{
			UIInfo uiInfo = GetUIInfo();
			if (!uiInfo)
				return super.GetActionNameScript(outName);

			outName = uiInfo.GetName();
		}

		outName = WidgetManager.Translate(outName, m_HitZoneGroupNames.GetHitZoneGroupName(m_aHitZoneGroups[0]));
		return super.GetActionNameScript(outName);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsStabilizedReason()
	{
		return m_eCannotPerformReason == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD;
	}

	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;

		super.DelayedInit(owner);

		//~ Make sure to check if the action is hidden if the entity is on fire
		if (m_bHideIfEntityOnFire)
		{
			IEntity root = owner.GetRootParent();

			//~ If has root and root is vehicle get that damage manager else use defualt damage manager
			if (root && Vehicle.Cast(root))
				m_OnFireCheckDamageManager = SCR_DamageManagerComponent.GetDamageManager(root);
			else
				m_OnFireCheckDamageManager = m_DamageManagerComponent;
		}
	}
}
