//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointDefendLarge : SCR_ScenarioFrameworkWaypointTimedDefend
{
	[Attribute(defvalue: "75", uiwidget: UIWidgets.Slider, desc: "Radius in m")]
	float m_fCompletionRadius;

	[Attribute(defvalue: "{FAD1D789EE291964}Prefabs/AI/Waypoints/AIWaypoint_Defend_Large.et", desc: "In Default, it will use specific prefab for each class, but here you can change it if you know what you are doing")]
	ResourceName m_sWaypointPrefab;

	[Attribute(defvalue: EAIWaypointCompletionType.All.ToString(), UIWidgets.ComboBox, "AI Waypoint Completion Type", "", ParamEnumArray.FromEnum(EAIWaypointCompletionType))]
	EAIWaypointCompletionType m_eAIWaypointCompletionType;

	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
	float m_fPriorityLevel;

	[Attribute("-1", UIWidgets.EditBox, "Minimal time to hold the waypoint before it completes")]
	float m_fHoldingTime;

	[Attribute("0", UIWidgets.Object, "Fast init - units will be spawned on their defensive locations")]
	bool m_bFastInit;

	//TODO: Add Array of SCR_DefendWaypointPreset once we can also pre-prepare it for World Editor attribute.

	//------------------------------------------------------------------------------------------------
	override void SetFastInit(bool enabled)
	{
		m_bFastInit = enabled;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetFastInit()
	{
		return m_bFastInit;
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