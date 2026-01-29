//! This class is used for keeping track of vehicles assigned to group
class SCR_AIGroupVehicle : Managed
{
	protected IEntity m_Vehicle;
	protected SCR_AIVehicleUsageComponent m_VehicleUsageComp;
	
	protected TurretCompartmentSlot m_TurretSlot;
	protected PilotCompartmentSlot m_DriverSlot;
	protected int m_iSubgroupHandleId;

	//------------------------------------------------------------------------------------------------	
	void SCR_AIGroupVehicle(notnull SCR_AIVehicleUsageComponent vehicleUsageComp)
	{
		m_Vehicle = vehicleUsageComp.GetOwner();
		m_VehicleUsageComp = vehicleUsageComp;
		m_iSubgroupHandleId = -1;
		
		// find pilot & turret slots
		SCR_BaseCompartmentManagerComponent compartmentMan = SCR_BaseCompartmentManagerComponent.Cast(m_Vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentMan)
			return;
			
		array<BaseCompartmentSlot> compartmentSlots = {};
		compartmentMan.GetCompartments(compartmentSlots);
		foreach (BaseCompartmentSlot slot : compartmentSlots)
		{
			TurretCompartmentSlot turretComp = TurretCompartmentSlot.Cast(slot);
			PilotCompartmentSlot pilotComp = PilotCompartmentSlot.Cast(slot);
			if (turretComp)
			{
				m_TurretSlot = turretComp;
			}
			if (pilotComp)
			{
				m_DriverSlot = pilotComp;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntity()
	{
		return m_Vehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIVehicleUsageComponent GetVehicleUsageComponent()
	{
		return m_VehicleUsageComp;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasWeapon()
	{
		return m_TurretSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanMove()
	{
		if (IsStatic())
			return false;
		
		if(SCR_AIVehicleUsability.VehicleCanMove(m_Vehicle) && !SCR_AIVehicleUsability.VehicleIsOnFire(m_Vehicle))
			return true;
	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsStatic()
	{
		return m_TurretSlot && !m_DriverSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	bool DriverIsConscious()
	{
		if (!m_DriverSlot)
			return false;
		
		IEntity driver = m_DriverSlot.GetOccupant();
		return SCR_AIDamageHandling.IsConscious(driver);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetDriver()
	{
		if (!m_DriverSlot)
			return null;
		return m_DriverSlot.GetOccupant();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSubgroupHandleId(int handleId = -1)
	{
		m_iSubgroupHandleId = handleId
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSubgroupHandleId()
	{
		return m_iSubgroupHandleId;
	}
};