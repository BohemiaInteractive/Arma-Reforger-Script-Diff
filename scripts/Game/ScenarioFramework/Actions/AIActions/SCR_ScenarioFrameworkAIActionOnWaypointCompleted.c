[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionOnWaypointCompleted : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "SlotWaypoint that spawns waypoint")];
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;

	[Attribute(desc: "Actions that will be executed upon provided waypoint is completed for provided AI group")];
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnWaypointCompleted;
	
	[Attribute(defvalue: "1", desc: "Remove the event on action completion")]
	bool m_bRemoveOnCompleted;
	
	AIWaypoint m_Waypoint;
	IEntity m_AISlotEntity;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_AIGroup targetAIGroup, IEntity entity)
	{
		if (entity)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layer && (SCR_ScenarioFrameworkSlotAI.Cast(layer) || SCR_ScenarioFrameworkSlotTaskAI.Cast(layer)))
				m_AISlotEntity = entity;
		}
		
		super.Init(targetAIGroup, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity getterEntity = entityWrapper.GetValue();
		if (!getterEntity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(getterEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
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
		
		m_AIGroup.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		layer.GetOnAllChildrenSpawned().Remove(OnWaypointSpawned);
		
		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_Waypoint = AIWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_AIGroup.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointCompleted(AIWaypoint waypoint)
	{
		if (waypoint != m_Waypoint)
			return;
		
		if (m_bRemoveOnCompleted)
			m_AIGroup.GetOnWaypointCompleted().Remove(OnWaypointCompleted);
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnWaypointCompleted)
		{
			action.Init(m_AISlotEntity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActionsOnWaypointCompleted;
	}
}