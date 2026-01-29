//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointPatrolHierarchy : SCR_ScenarioFrameworkWaypointScripted
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.Slider, desc: "Radius in m")]
	float m_fCompletionRadius;

	[Attribute(defvalue: "{FBA8DC8FDA0E770D}Prefabs/AI/Waypoints/AIWaypoint_Patrol_Hierarchy.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: EAIWaypointCompletionType.Any.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", "", ParamEnumArray.FromEnum(EAIWaypointCompletionType))]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
	float m_fPriorityLevel;

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