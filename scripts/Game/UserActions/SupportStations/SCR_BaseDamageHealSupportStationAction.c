//! Allows healing of action owner (not user)
class SCR_BaseDamageHealSupportStationAction : SCR_BaseUseSupportStationAction
{
	[Attribute(desc: "Used to get the damage state such as Critical, minor ect")]
	protected ref SCR_DamageIntensityHolder m_DamageIntensityHolder;
	
	SCR_DamageManagerComponent m_DamageManagerComponent;
	
	//~ A list of hitZones to heal
	protected ref array<HitZone> m_aHitZonesToHeal = {};
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		m_DamageManagerComponent = SCR_DamageManagerComponent.GetDamageManager(owner);
		
		//~ Set hitZones to Repair
		if (m_DamageManagerComponent)
			SetHitZonesToHeal();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get HitZones this heal support station action can heal
	\param[out] Array of hitzones to heal
	\return Number of hitzones to heal
	*/
	int GetHitZonesToHeal(notnull out array<HitZone> hitZones)
	{
		hitZones.Copy(m_aHitZonesToHeal);
		return hitZones.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetHitZonesToHeal()
	{
		m_aHitZonesToHeal.Clear();
		
		if (!m_DamageManagerComponent)
			return;
		
		m_DamageManagerComponent.GetAllHitZonesInHierarchy(m_aHitZonesToHeal);
		
		if (m_aHitZonesToHeal.Count() > 1)
		{
			HitZone defaultHitZone = m_DamageManagerComponent.GetDefaultHitZone();
			
			//~ Move default hitZone to end
			if (defaultHitZone)
			{
				int defaultHitZoneIndex = m_aHitZonesToHeal.Find(defaultHitZone);
				if (m_aHitZonesToHeal.IsIndexValid(defaultHitZoneIndex))
				{
					m_aHitZonesToHeal.RemoveOrdered(defaultHitZoneIndex);
					m_aHitZonesToHeal.InsertAt(defaultHitZone, m_aHitZonesToHeal.Count() -1);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DamageManagerComponent GetActionDamageManager()
	{
		return m_DamageManagerComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{
		if (!m_DamageIntensityHolder || m_aHitZonesToHeal.IsEmpty() || (!m_bCanPerform && m_eCannotPerformReason != ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD && m_eCannotPerformReason != ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY))
			return string.Empty;
		
		SCR_UIName uiInfo = m_DamageIntensityHolder.GetValidIntensityUIInfo(m_aHitZonesToHeal);
		if (uiInfo)
			return uiInfo.GetName();
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! return the Health code color for nearby interactions
	SCR_ENearbyInteractionContextColors GetHealthStatus()
	{
		// Hitzones to heal are cached but if the entity owning them
		// gets deleted we'd get a VME inside GetValidIntensityType
		// because all hitzones would be null.
		if (!m_DamageManagerComponent)
			return SCR_ENearbyInteractionContextColors.DEFAULT;
		
		int healthState = m_DamageIntensityHolder.GetValidIntensityType(m_aHitZonesToHeal);
		
		if (healthState == SCR_EDamageIntensityType.NO_DAMAGE)
			return SCR_ENearbyInteractionContextColors.DEFAULT;
		
		if (IsInProgress())
			return SCR_ENearbyInteractionContextColors.HEALING;
		
		if (IsStabilizedReason())
			return SCR_ENearbyInteractionContextColors.STABLE;
		
		if (healthState < SCR_EDamageIntensityType.HIGH)
			return SCR_ENearbyInteractionContextColors.MEDIUM;
		
		return SCR_ENearbyInteractionContextColors.SEVERE;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsStabilizedReason()
	{
		return m_eCannotPerformReason == ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD;
	}
}
