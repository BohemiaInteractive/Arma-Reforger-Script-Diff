[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIAddWaypoint : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "SlotWaypoint that spawns waypoint - Use GetLayerBase")];
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "True checked, this waypoint will be added on a first position. Otherwise it will append it at the end of the waypoint queue.")];
	bool m_bAddOnTopOfQueue;
	
	[Attribute(desc: "True checked, previous waypoints should be cleared.")];
	bool m_bClearPreviousWaypoints;
	
	AIWaypoint m_Waypoint;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_Waypoint = AIWaypoint.Cast(entity);
		if (!m_Waypoint)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
				return;
			}
			
			IEntity spawnedEntity = layer.GetSpawnedEntity();
			if (!spawnedEntity)
			{
				layer.GetOnAllChildrenSpawned().Insert(OnWaypointSpawned);
				return;
			}
			
			m_Waypoint = AIWaypoint.Cast(spawnedEntity);
			if (!m_Waypoint)
			{
				Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
				return;
			}
		}
		
		if (m_bClearPreviousWaypoints)
			ClearCurrentWaypoints();
		
		if (m_bAddOnTopOfQueue)
			m_AIGroup.AddWaypointAt(m_Waypoint, 0);
		else
			m_AIGroup.AddWaypoint(m_Waypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearCurrentWaypoints()
	{
		array<AIWaypoint> currentWaypoints = {};
		m_AIGroup.GetWaypoints(currentWaypoints);
		foreach (AIWaypoint waypoint : currentWaypoints)
		{
			m_AIGroup.RemoveWaypointFromGroup(waypoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		layer.GetOnAllChildrenSpawned().Remove(OnWaypointSpawned);
		
		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
			return;
		
		m_Waypoint = AIWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		if (m_bClearPreviousWaypoints)
			ClearCurrentWaypoints();
				
		if (m_bAddOnTopOfQueue)
			m_AIGroup.AddWaypointAt(m_Waypoint, 0);
		else
			m_AIGroup.AddWaypoint(m_Waypoint);
	}
}