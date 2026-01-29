[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_PowerComponentClass : ScriptGameComponentClass
{
}

class SCR_PowerComponent : ScriptGameComponent
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Should the engine output be reduced when there's no power (batteries destroyed)")]
	private bool m_bShouldApplyNoPowerPenalty;
	
	[Attribute( defvalue: "0.4", uiwidget: UIWidgets.EditBox, desc: "The multiplier to apply to the engine output in case the vehicle has no power (batteries destroyed)")]
	private float m_fNoPowerMultiplier;
	
	private ref ScriptInvoker m_EventBatteryStateChanged;
	private ref array<HitZone> m_aBatteryHitZones;
	private bool m_bHasPower = true;

	//------------------------------------------------------------------------------------------------
	//! \return
	bool HasPower()
	{
		return m_bHasPower; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool ShouldApplyNoPowerPenalty()
	{
		return m_bShouldApplyNoPowerPenalty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetNoPowerMultiplier()
	{
		return m_fNoPowerMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] createNew
	//! \return
	ScriptInvoker GetEventBatteryStateChanged(bool createNew = true)
	{
		if (createNew && !m_EventBatteryStateChanged)
			m_EventBatteryStateChanged = new ScriptInvoker();
		
		return m_EventBatteryStateChanged;
	}
	
	//!------------------------------------------------------------------------------------------------
	//! If there are battery colliders, check if at leat one of them is not destroyed
	void UpdatePowerState()
	{
		// Power is available if there are no colliders
		bool hasPower = !m_aBatteryHitZones || m_aBatteryHitZones.IsEmpty();
	
		if (m_aBatteryHitZones)
		{
			foreach (HitZone hitZone : m_aBatteryHitZones)
			{
				if (hitZone.GetDamageState() != EDamageState.DESTROYED)
				{
					hasPower = true;
					break;
				}
			}
		}
		
		if (m_bHasPower == hasPower)
			return;
		
		m_bHasPower = hasPower;
		
		if (m_EventBatteryStateChanged)
			m_EventBatteryStateChanged.Invoke(hasPower);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitZone
	void RegisterBatteryHitZone(notnull HitZone hitZone)
	{
		if (!m_aBatteryHitZones)
			m_aBatteryHitZones = {};
		
		m_aBatteryHitZones.Insert(hitZone);
		
		// Trigger power state update if new battery has different state
		bool hasPower = hitZone.GetDamageState() != EDamageState.DESTROYED;
		if (m_bHasPower != hasPower)
			UpdatePowerState();
	}
}
