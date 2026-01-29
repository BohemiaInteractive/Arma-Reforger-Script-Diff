//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointSuppress : SCR_ScenarioFrameworkWaypointTimed
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Radius in m")]
	float m_fCompletionRadius;

	[Attribute(defvalue: "{ED8277F35B46B4AA}Prefabs/AI/Waypoints/AIWaypoint_Suppress.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: EAIWaypointCompletionType.All.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", "", ParamEnumArray.FromEnum(EAIWaypointCompletionType))]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
	float m_fPriorityLevel;

	[Attribute("20", UIWidgets.EditBox, "Minimal time to hold the waypoint before it completes")]
	float m_fHoldingTime;
	
	[Attribute("1.5", UIWidgets.EditBox, desc: "Height of suppression volume above waypoint center")]
	float m_fSuppressionHeight;
	
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_SuppressWaypoint waypointSuppress = SCR_SuppressWaypoint.Cast(waypointEntity);
		if (!waypointSuppress)
			return;

		waypointSuppress.SetSuppressionHeight(m_fSuppressionHeight);
	}

	//------------------------------------------------------------------------------------------------
	override void SetWaypointHoldingTime(float time)
	{
		m_fHoldingTime = time;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWaypointHoldingTime()
	{
		return m_fHoldingTime;
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
	override void SetWaypointPriorityLevel(float priority)
	{
		m_fPriorityLevel = priority;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWaypointPriorityLevel()
	{
		return m_fPriorityLevel;
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