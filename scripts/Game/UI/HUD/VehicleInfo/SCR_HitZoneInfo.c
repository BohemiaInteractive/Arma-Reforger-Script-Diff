//#define SHOW_DMG_INDICATORS_WARNING
//#define SHOW_DMG_INDICATORS_ERROR

class SCR_HitZoneInfo : SCR_BaseVehicleInfo
{
	[Attribute("", uiwidget: UIWidgets.Auto)]
	protected ref array<string> m_aHitZoneNames;

	protected ref array<HitZone> m_aHitZones;

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		#ifdef SHOW_DMG_INDICATORS_ERROR
		return EVehicleInfoState.ERROR;
		#endif
		
		#ifdef SHOW_DMG_INDICATORS_WARNING
		return EVehicleInfoState.WARNING;
		#endif
		
		if (!m_aHitZones)
			return EVehicleInfoState.DISABLED;

		m_bIsBlinking = false;

		EDamageState damageState;
		EVehicleInfoState state = EVehicleInfoState.DISABLED;
		SCR_WheelHitZone wheelHitZone;
		foreach (HitZone hitZone : m_aHitZones)
		{
			if (!hitZone)
				continue;
			
			// Ignore spare wheels
			wheelHitZone = SCR_WheelHitZone.Cast(hitZone);
			if (wheelHitZone && wheelHitZone.GetWheelIndex() < 0)
				continue;

			// On fire
			if (hitZone.GetDamageOverTime(EDamageType.FIRE) > 0)
			{
				m_bIsBlinking = true;
				return EVehicleInfoState.ERROR;
			}

			// Destroyed or damaged
			damageState = hitZone.GetDamageState();
			if (damageState > EDamageState.DESTROYED)
				state = EVehicleInfoState.WARNING;
			else if (damageState == EDamageState.DESTROYED)
				return EVehicleInfoState.ERROR;
		}

		return state;
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return false;

		m_aHitZones = {};

		// Just get default hitzone if none are provided
		if (m_aHitZoneNames.IsEmpty())
		{
			HitZone hitZone = damageManager.GetDefaultHitZone();
			if (hitZone)
				m_aHitZones = {hitZone};
		}
		else
		{
			// Get lowercase hitzone names
			array<string> hitzoneNames = {};
			foreach (string name : m_aHitZoneNames)
			{
				name.ToLower();
				hitzoneNames.Insert(name);
			}

			// Find all matching hitzones
			m_aHitZones = {};
			array<HitZone> hitZones = {};
			damageManager.GetAllHitZonesInHierarchy(hitZones);
			string name;
			foreach (HitZone hitZone : hitZones)
			{
				name = hitZone.GetName();
				name.ToLower();
				if (hitzoneNames.Contains(name))
					m_aHitZones.Insert(hitZone);
			}
		}

		if (m_aHitZones.IsEmpty())
			m_aHitZones = null;

		// Terminate if there are no hitzones
		if (!m_aHitZones)
			return false;

		return super.DisplayStartDrawInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		m_aHitZones = null;
	}
};
