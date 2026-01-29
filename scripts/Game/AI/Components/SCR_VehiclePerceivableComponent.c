class SCR_VehiclePerceivableComponentClass : VehiclePerceivableComponentClass
{
}

class SCR_VehiclePerceivableComponent : VehiclePerceivableComponent
{
	protected SCR_DamageManagerComponent m_DamageMgr;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_DamageMgr = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));

		if (m_DamageMgr)
		{
			m_DamageMgr.GetOnDamageOverTimeAdded().Insert(OnDamageOverTimeAdded);
			m_DamageMgr.GetOnDamageOverTimeRemoved().Insert(OnDamageOverTimeRemoved);			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_VehiclePerceivableComponent()
	{
		if (m_DamageMgr)
		{
			m_DamageMgr.GetOnDamageOverTimeAdded().Remove(OnDamageOverTimeAdded);
			m_DamageMgr.GetOnDamageOverTimeRemoved().Remove(OnDamageOverTimeRemoved);			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] dType
	//! \param[in] dps
	//! \param[in] hz
	void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		// Consider the vehicle disarmed once it starts burning
		if (dType == EDamageType.FIRE)
			SetDisarmed(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] dType
	//! \param[in] hz
	void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		if (dType == EDamageType.FIRE)
			SetDisarmed(false);
	}
}
