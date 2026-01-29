class SCR_UnloadSuppliesWaypointClass : SCR_SuppliesTransferWaypointClass
{
}

class SCR_UnloadSuppliesWaypoint : SCR_SuppliesTransferWaypoint
{
	//------------------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_UnloadSuppliesWaypointState(groupUtilityComp, this);
	}
}

class SCR_UnloadSuppliesWaypointState : SCR_SuppliesTransferWaypointState
{
	//------------------------------------------------------------------------------------------------
	void SCR_UnloadSuppliesWaypointState(notnull SCR_AIGroupUtilityComponent utility, SCR_AIWaypoint waypoint)
	{
		SCR_UnloadSuppliesWaypoint wp = SCR_UnloadSuppliesWaypoint.Cast(waypoint);
		if (!wp)
			return;

		InitSupplyTransfer(utility, wp, EResourceGeneratorID.DEFAULT, EResourceGeneratorID.VEHICLE_UNLOAD, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Unload()
	{
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode || !gamemode.IsMaster())
			return;

		SCR_UnloadSuppliesWaypoint wp = SCR_UnloadSuppliesWaypoint.Cast(m_Waypoint);
		if (!wp)
			return;

		if (!m_ResourceComponent || !m_ResourceGenerator || !m_ResourceConsumer)
		{
			Print("Missing resource generator, consumer or component!", LogLevel.WARNING);
			if (m_Utility && m_Utility.m_Owner)
				m_Utility.OnMoveFailed(EMoveError.UNKNOWN, GetVehicleFromGroupLeader(m_Utility), true, m_Utility.m_Owner.GetOrigin());

			return;
		}

		SCR_ResourceContainer container = m_ResourceComponent.GetContainer(RESOURCE_TYPE);
		float maxResourceValue = container.GetMaxResourceValue();

		SCR_ResourceGenerationResponse response = m_ResourceGenerator.RequestAvailability(maxResourceValue);

		GetResourceValues(m_fCurrentResource, m_fMaxStoredResource, m_fCurrentTransferValue);

		m_ResourceConsumer.RequestConsumtion(m_fCurrentTransferValue);
		m_ResourceGenerator.RequestGeneration(m_fCurrentTransferValue);

		bool isStorageFull = (m_ResourceGenerator.GetAggregatedMaxResourceValue() - m_ResourceGenerator.GetAggregatedResourceValue()) <= 0;
		if (isStorageFull || container.GetResourceValue() == 0)
		{
			if (m_Utility && m_Utility.m_Owner)
				m_Utility.m_Owner.CompleteWaypoint(m_Waypoint);
		}
		else
		{
			// Schedule the next supply unload after a delay, unless the container is full or the truck empty
			GetGame().GetCallqueue().CallLater(Unload, wp.GetTransferDelay() * 1000, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSelected()
	{
		super.OnSelected();

		Unload();
	}
}
