class SCR_LoadSuppliesWaypointClass : SCR_SuppliesTransferWaypointClass
{
}

class SCR_LoadSuppliesWaypoint : SCR_SuppliesTransferWaypoint
{
	[Attribute("0.5", desc: "Ratio of supplies fulfillment to consider load waypoint completed in a case of supply source being empty", params: "0 1 0.01")]
	protected float m_fLoadFulfillmentThresholdWhenSourceEmpty;

	//------------------------------------------------------------------------------------------------
	//! \return Percentage of load for load waypoint considered completed in case of supply source being empty
	float GetLoadFulfillmentThresholdWhenSourceEmpty()
	{
		return m_fLoadFulfillmentThresholdWhenSourceEmpty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] Percentage value of load for load waypoint considered completed in case of supply source being empty
	void SetLoadFulfillmentThresholdWhenSourceEmpty(float value)
	{
		m_fLoadFulfillmentThresholdWhenSourceEmpty = Math.Clamp(value, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_LoadSuppliesWaypointState(groupUtilityComp, this);
	}
}

class SCR_LoadSuppliesWaypointState : SCR_SuppliesTransferWaypointState
{
	//------------------------------------------------------------------------------------------------
	void SCR_LoadSuppliesWaypointState(notnull SCR_AIGroupUtilityComponent utility, SCR_AIWaypoint waypoint)
	{
		SCR_LoadSuppliesWaypoint wp = SCR_LoadSuppliesWaypoint.Cast(waypoint);
		if (!wp)
			return;

		InitSupplyTransfer(utility, wp, EResourceGeneratorID.VEHICLE_LOAD, EResourceGeneratorID.DEFAULT, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void Load()
	{
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode || !gamemode.IsMaster())
			return;

		SCR_LoadSuppliesWaypoint wp = SCR_LoadSuppliesWaypoint.Cast(m_Waypoint);
		if (!wp)
			return;

		if (!m_ResourceComponent || !m_ResourceGenerator || !m_ResourceConsumer)
			return;

		SCR_ResourceContainer container = m_ResourceComponent.GetContainer(RESOURCE_TYPE);
		float maxResourceValue = container.GetMaxResourceValue();

		SCR_ResourceConsumtionResponse response = m_ResourceConsumer.RequestAvailability(maxResourceValue);

		GetResourceValues(m_fCurrentResource, m_fMaxStoredResource, m_fCurrentTransferValue);

		if (m_fCurrentTransferValue != 0)
		{
			m_ResourceConsumer.RequestConsumtion(m_fCurrentTransferValue);
			m_ResourceGenerator.RequestGeneration(m_fCurrentTransferValue);
		}

		if (container.GetResourceValue() >= container.GetMaxResourceValue()
		|| (m_ResourceConsumer.GetAggregatedResourceValue() == 0 && container.GetResourceValue() >= container.GetMaxResourceValue() * wp.GetLoadFulfillmentThresholdWhenSourceEmpty()))
		{
			if (m_Utility && m_Utility.m_Owner)
				m_Utility.m_Owner.CompleteWaypoint(m_Waypoint);
		}
		else
		{
			// Schedule the next supply load after a delay, unless the container is empty or the truck is full
			GetGame().GetCallqueue().CallLater(Load, wp.GetTransferDelay() * 1000, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSelected()
	{
		super.OnSelected();
		Load();
	}
}
