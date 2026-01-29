class SCR_AIGroupVehicleManager : Managed
{
	protected ref array<ref SCR_AIGroupVehicle> m_aGroupVehicles = {};
	
	//---------------------------------------------------------------------------------------------------
	SCR_AIGroupVehicle FindVehicle(IEntity vehicleEntity)
	{
		foreach (SCR_AIGroupVehicle groupVehicle : m_aGroupVehicles)
		{
			if (groupVehicle.GetEntity() == vehicleEntity)
				return groupVehicle;
		}
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	SCR_AIGroupVehicle FindVehicle(SCR_AIVehicleUsageComponent vehicleUsageComp)
	{
		foreach (SCR_AIGroupVehicle groupVehicle : m_aGroupVehicles)
		{
			if (groupVehicle.GetVehicleUsageComponent() == vehicleUsageComp)
				return groupVehicle;
		}
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	SCR_AIGroupVehicle FindVehicleBySubgroupId(int vehicleHandlerId)
	{
		foreach (SCR_AIGroupVehicle groupVehicle : m_aGroupVehicles)
		{
			if (groupVehicle.GetSubgroupHandleId() == vehicleHandlerId)
				return groupVehicle;
		}
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	void GetAllVehicles(array<ref SCR_AIGroupVehicle> outAllVehicles)
	{
		outAllVehicles.Clear();
		foreach(auto groupVehicle : m_aGroupVehicles)
			outAllVehicles.Insert(groupVehicle);
	}
	
	//---------------------------------------------------------------------------------------------------
	void GetAllVehicleEntities(array<IEntity> outAllVehicles)
	{
		outAllVehicles.Clear();
		foreach(SCR_AIGroupVehicle groupVehicle : m_aGroupVehicles)
			outAllVehicles.Insert(groupVehicle.GetEntity());
	}
	
	//---------------------------------------------------------------------------------------------------
	int GetVehiclesCount()
	{
		return m_aGroupVehicles.Count();
	}
	
	//---------------------------------------------------------------------------------------------------
	SCR_AIGroupVehicle TryAddVehicle(notnull SCR_AIVehicleUsageComponent vehicleUsageComp)
	{	
		SCR_AIGroupVehicle groupVehicle = FindVehicle(vehicleUsageComp);
		if (groupVehicle)
			return groupVehicle;
		
		SCR_AIGroupVehicle newGroupVehicle = new SCR_AIGroupVehicle(vehicleUsageComp);
			m_aGroupVehicles.Insert(newGroupVehicle);
		
		return newGroupVehicle;
	}
	
	//---------------------------------------------------------------------------------------------------
	bool RemoveVehicle(notnull SCR_AIVehicleUsageComponent vehicleUsageComp)
	{
		foreach (int index, SCR_AIGroupVehicle groupVehicle : m_aGroupVehicles)
		{
			if (groupVehicle.GetVehicleUsageComponent() == vehicleUsageComp)
			{
				m_aGroupVehicles.Remove(index);
				return true;
			}
		}
		return false;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Called from SCR_AIGroupUtilityComponent
	//! Removes vehicle of removed agent from group usage if there is no other member of group using the same vehicle
	//! \param[in] agent
	//! \param[in] groupUsingVehicle
	void OnAgentRemoved(notnull AIAgent removedAgent, notnull SCR_AIGroup agentFormerGroup)
	{
		// Note: removedAgent is not the group member anymore
		ChimeraCharacter agentEntity = ChimeraCharacter.Cast(removedAgent.GetControlledEntity());
		if (!agentEntity || !agentEntity.IsInVehicle())
			return;
		
		CompartmentAccessComponent compAcc = agentEntity.GetCompartmentAccessComponent();
		if (!compAcc)
			return;
		
		IEntity vehicleOfAgent = compAcc.GetVehicleIn(agentEntity);
		if (!vehicleOfAgent)
			return;
		
		SCR_AIVehicleUsageComponent usage = SCR_AIVehicleUsageComponent.Cast(vehicleOfAgent.FindComponent(SCR_AIVehicleUsageComponent));
		if (!usage)
			return;
		
		if (!usage.IsOccupiedByGroup(agentFormerGroup))
			RemoveVehicle(usage);
	}
};