//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointCycle : SCR_ScenarioFrameworkWaypoint
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in m")]
	float m_fCompletionRadius;

	[Attribute(defvalue: EAIWaypointCompletionType.All.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", "", ParamEnumArray.FromEnum(EAIWaypointCompletionType))]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
	float m_fPriorityLevel;

	[Attribute(defvalue: "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "From this list, layers will be used and their waypoints processed")]
	ref array<string> 	m_aLayersWithWaypointsToCycle;

	[Attribute(defvalue: "-1", desc: "Decides how many times the cycled waypoint will run for specific group. -1 means infinite. ", UIWidgets.Graph, "-1 inf 1")]
	int m_iRerunCounter;

	[Attribute("Use random order")]
	bool m_bUseRandomOrder;

	ref ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod> m_OnAllWaypointsSpawned;
	ref array<AIWaypoint> m_aWaypointsWithoutCycle = {};
	ref array<SCR_ScenarioFrameworkSlotWaypoint> m_aSlotWaypoints = {};
	AIWaypointCycle m_CycleWaypoint;
	int m_iCurrentlySpawnedWaypoints;
	bool m_bInitiated;

	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		m_CycleWaypoint = AIWaypointCycle.Cast(waypointEntity);
		if (!m_CycleWaypoint)
			return;

		array<AIWaypoint> waypointsWithoutCycle = {};
		array<SCR_ScenarioFrameworkLayerBase> layerBases = {};
		array<SCR_ScenarioFrameworkLayerBase> layerChildren = {};
		SCR_ScenarioFrameworkSlotWaypoint slotWaypoint;

		// If no layer or slot is specified, it will assume that it is in the layer with other waypoints and will attempt to fetch them from there
		// It is intentionally designed that way to make whole setup easier
		if (m_aLayersWithWaypointsToCycle.IsEmpty())
		{
			SCR_ScenarioFrameworkLayerBase layerBase = m_SlotWaypoint.GetParentLayer();
			if (layerBase)
				m_aLayersWithWaypointsToCycle.Insert(layerBase.GetName());
		}

		foreach (string waypointSetLayer : m_aLayersWithWaypointsToCycle)
		{
			IEntity layerEntity = GetGame().GetWorld().FindEntityByName(waypointSetLayer);
			if (!layerEntity)
				continue;

			SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase && !SCR_ScenarioFrameworkSlotWaypoint.Cast(layerBase))
			{
				if (layerBases.Contains(layerBase))
					continue;

				layerChildren = layerBase.GetChildrenEntities();
				foreach (SCR_ScenarioFrameworkLayerBase child : layerChildren)
				{
					slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(child);
					if (slotWaypoint && !m_aSlotWaypoints.Contains(slotWaypoint))
						m_aSlotWaypoints.Insert(slotWaypoint);
				}
			}
			else
			{
				slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkSlotWaypoint));
				if (slotWaypoint && !m_aSlotWaypoints.Contains(slotWaypoint))
						m_aSlotWaypoints.Insert(slotWaypoint);
			}
		}

		int waypointCount = m_aSlotWaypoints.Count();
		foreach (SCR_ScenarioFrameworkSlotWaypoint slotWaypointToProcess : m_aSlotWaypoints)
		{
			if (slotWaypointToProcess.GetIsInitiated())
				m_iCurrentlySpawnedWaypoints++;
			else
				slotWaypointToProcess.GetOnAllChildrenSpawned().Insert(CheckWaypointsAfterInit);

			if (waypointCount == m_iCurrentlySpawnedWaypoints)
				ProcessWaypoints(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckWaypointsAfterInit(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_iCurrentlySpawnedWaypoints++;
		if (m_iCurrentlySpawnedWaypoints == m_aSlotWaypoints.Count())
			ProcessWaypoints(null);

		layer.GetOnAllChildrenSpawned().Remove(CheckWaypointsAfterInit);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] layer
	protected void ProcessWaypoints(SCR_ScenarioFrameworkLayerBase layer)
	{
		AIWaypoint waypoint;
		foreach (SCR_ScenarioFrameworkSlotWaypoint slotWaypoint : m_aSlotWaypoints)
		{
			waypoint = AIWaypoint.Cast(slotWaypoint.GetSpawnedEntity());
			if (waypoint && !AIWaypointCycle.Cast(waypoint))
				m_aWaypointsWithoutCycle.Insert(waypoint);
		}

		int wpCount = m_aWaypointsWithoutCycle.Count();
		if (m_bUseRandomOrder && wpCount > 1)
		{
			foreach (AIWaypoint wp : m_aWaypointsWithoutCycle)
			{
				ShuffleWaypointArray(m_aWaypointsWithoutCycle);
			}
		}

		m_CycleWaypoint.SetRerunCounter(m_iRerunCounter);
		m_CycleWaypoint.SetWaypoints(m_aWaypointsWithoutCycle);
		m_bInitiated = true;
		InvokeAllWaypointsSpawned();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShuffleWaypointArray(notnull array<AIWaypoint> arr)
	{
	    int n = arr.Count();
	    for (int i = n - 1; i > 0; i--)
	    {
	        int j = Math.RandomInt(0, i + 1);

			arr.SwapItems(i, j);
	    }
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerScenarioFrameworkLayer GetOnAllWaypointsSpawned()
	{
		if (!m_OnAllWaypointsSpawned)
			m_OnAllWaypointsSpawned = new ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod>();

		return m_OnAllWaypointsSpawned;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void InvokeAllWaypointsSpawned()
	{
		if (m_OnAllWaypointsSpawned && m_SlotWaypoint)
			m_OnAllWaypointsSpawned.Invoke(m_SlotWaypoint);
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointCompletionRadius(float radius)
	{
		m_fCompletionRadius = radius;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWaypointCompletionRadius()
	{
		return m_fCompletionRadius;
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointCompletionType(EAIWaypointCompletionType type)
	{
		m_eAIWaypointCompletionType = type;
	}

	//------------------------------------------------------------------------------------------------
	override EAIWaypointCompletionType GetWaypointCompletionType()
	{
		return m_eAIWaypointCompletionType;
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointPrefab(ResourceName prefab)
	{
		m_sWaypointPrefab = prefab;
	}

	//------------------------------------------------------------------------------------------------
	override ResourceName GetWaypointPrefab()
	{
		return m_sWaypointPrefab;
	}
}